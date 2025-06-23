// Cashmere - a distributed conflict-free replicated database.
// Copyright (C) 2025 Aeliton G. Silva
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include "broker.h"
#include <cassert>
#include <utility>

namespace Cashmere
{

IdConnectionInfoMap UpdateProvides(IdConnectionInfoMap provides);

Broker::Broker()
{
  _connections.push_back({});
}

Broker::~Broker() = default;

Connection Broker::connect(Connection conn)
{
  Port port = _connections.size();
  _connections.push_back(conn);
  refreshConnections(port);
  return {ptr(), port, clock(), UpdateProvides(provides(port))};
}

Port Broker::connect(BrokerBasePtr remote)
{
  if (!remote) {
    return -1;
  }
  const Port port = _connections.size();
  _connections.push_back(
    remote->connect({ptr(), port, clock(), UpdateProvides(provides(port))})
  );
  auto& conn = _connections.at(port);

  auto thisEntries = query(conn.version(), port);
  auto brokerEntries = conn.entries(clock());

  if (brokerEntries.size() > 0) {
    BrokerBase::insert(brokerEntries, port);
  }
  if (thisEntries.size() > 0) {
    conn.insert(thisEntries);
  }

  refreshConnections(port);

  return port;
}

bool Broker::refresh(const Connection& conn, Port port)
{
  if (port <= 0 || _connections.size() <= port) {
    return false;
  }
  _connections[port] = conn;

  refreshConnections(port);
  return true;
}

Clock Broker::insert(const Entry& data, Port port)
{
  if (port < 0 || port >= _connections.size()) {
    return Clock();
  }

  setClock(clock().merge(data.clock));
  auto& conn = _connections.at(port);
  conn.provides()[data.entry.id].version = clock();

  for (size_t i = 0; i < _connections.size(); ++i) {
    if (i == port) {
      continue;
    }
    auto& ctx = _connections[i];
    ctx.insert(data);
  }
  return clock();
}

IdConnectionInfoMap Broker::provides(Port to) const
{
  IdConnectionInfoMap out;
  for (size_t i = 0; i < _connections.size(); i++) {
    if (i == to) {
      continue;
    }
    auto& conn = _connections[i];
    if (conn.active()) {
      for (auto& [id, data] : conn.provides()) {
        auto it = std::as_const(out).find(id);
        if (it == out.cend() || data.distance < it->second.distance) {
          out[id] = data;
        }
      }
    }
  }
  if (id() > 0) {
    out[id()] = {0, clock()};
  }
  return out;
}

EntryList Broker::query(const Clock& from, Port ignore) const
{
  if (id() > 0) {
    return query(from);
  }
  for (size_t i = 1; i < _connections.size(); i++) {
    auto conn = _connections[i];
    if (i == ignore || conn.provides().empty()) {
      continue;
    }
    if (conn.active()) {
      return conn.entries(from);
    }
  }
  return {};
}

IdClockMap Broker::versions() const
{
  IdClockMap out;
  for (auto& conn : _connections) {
    for (auto& [id, data] : conn.provides()) {
      out[id] = out[id].merge(data.version);
    }
  }
  return out;
}

Port Broker::disconnect(Port port)
{
  if (port < 0 || _connections.size() <= port) {
    return -1;
  }
  auto& conn = _connections.at(port);
  if (conn.active()) {
    conn.disconnect();
    refreshConnections();
    return port;
  }
  return -1;
}

Clock Broker::clock() const
{
  return _connections.front().version();
}

BrokerBasePtr Broker::ptr()
{
  return this->shared_from_this();
}

void Broker::setClock(const Clock& clock)
{
  _connections.front().version() = clock;
}

bool ConnectionInfo::operator==(const ConnectionInfo& other) const
{
  return std::tie(distance, version) == std::tie(other.distance, other.version);
}

bool ConnectionInfo::operator<(const ConnectionInfo& other) const
{
  return std::tie(distance, version) < std::tie(other.distance, other.version);
}

IdConnectionInfoMap UpdateProvides(IdConnectionInfoMap provides)
{
  for (auto& [id, data] : provides) {
    ++data.distance;
  }
  return provides;
}

Id Broker::id() const
{
  return 0;
}
std::set<Port> Broker::connectedPorts() const
{
  std::set<Port> connected;
  for (size_t i = 1; i < _connections.size(); i++) {
    connected.insert(i);
  }
  return connected;
}

void Broker::refreshConnections(Port ignore)
{
  for (size_t i = 1; i < _connections.size(); i++) {
    if (i == ignore) {
      continue;
    }
    auto& conn = _connections.at(i);
    conn.refresh(
      {ptr(), static_cast<Port>(i), clock(), UpdateProvides(provides(i))}
    );
  }
}

}
