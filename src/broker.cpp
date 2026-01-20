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
#include "cashmere/broker.h"
#include "cashmere/brokerbase.h"
#include <cassert>
#include <limits>
#include <utility>

namespace Cashmere
{

IdConnectionInfoMap UpdateProvides(SourcesMap provides);

Broker::Broker(Id id)
  : BrokerBase(id)
{
  _connections.push_back(Connection{});
}

Broker::~Broker() = default;

Connection Broker::connect(Connection conn)
{
  Connection out(stub());

  if (conn.source() == 0) {
    if (!conn.valid()) {
      out.source() = -1;
      return out;
    }

    out.source() = _connections.size();
    _connections.push_back(conn);

    auto& conn = _connections.at(out.source());

    auto data = stub();
    data.source() = out.source();
    data.clock() = clock();
    data.provides() = UpdateProvides(sources(out.source()));
    conn.connect(data);

    auto thisEntries = query(conn.clock(), out.source());
    auto brokerEntries = conn.query(clock());

    if (brokerEntries.size() > 0) {
      BrokerBase::insert(brokerEntries, out.source());
    }
    if (thisEntries.size() > 0) {
      conn.insert(thisEntries);
    }

    refreshConnections(out.source());
  } else {
    out.source() = _connections.size();

    auto version = clock();
    for (auto& [id, info] : conn.provides()) {
      info.clock = info.clock.merge(version);
    }

    _connections.push_back(conn);
    refreshConnections(out.source());
    out.clock() = version;
    out.provides() = UpdateProvides(sources(out.source()));
  }
  return out;
}

bool Broker::refresh(const Connection& data, Source sender)
{
  if (sender <= 0 || static_cast<size_t>(sender) >= _connections.size()) {
    return false;
  }

  _connections[sender].source() = data.source();
  _connections[sender].clock() = data.clock();
  _connections[sender].provides() = data.provides();

  refreshConnections(sender);
  return true;
}

Clock Broker::insert(const Entry& data, Source source)
{
  if (source < 0 || static_cast<size_t>(source) >= _connections.size()) {
    return Clock{{0, 0}};
  }

  setClock(clock().merge(data.clock));
  auto& conn = _connections.at(source);
  conn.provides()[data.entry.id].clock = clock();

  for (size_t i = 0; i < _connections.size(); ++i) {
    if (i == static_cast<size_t>(source)) {
      continue;
    }
    auto& ctx = _connections[i];
    ctx.insert(data);
  }
  return clock();
}

SourcesMap Broker::sources(Source sender) const
{
  SourcesMap out;
  for (size_t i = 0; i < _connections.size(); i++) {
    if (i > 0 && i == static_cast<size_t>(sender)) {
      continue;
    }
    const auto& conn = _connections[i];
    if (!conn.valid()) {
      continue;
    }
    const auto& sources = conn.provides();
    if (sources.empty()) {
      continue;
    }
    out[i] = _connections[i].provides();
  }
  return out;
}

EntryList Broker::query(const Clock& from, Source sender) const
{
  for (size_t i = 1; i < _connections.size(); i++) {
    auto conn = _connections[i];
    if (i == static_cast<size_t>(sender) || conn.provides().empty()) {
      continue;
    }
    if (conn.valid()) {
      return conn.query(from);
    }
  }
  return {};
}

IdClockMap Broker::versions() const
{
  IdClockMap out;
  for (auto& conn : _connections) {
    for (auto& [id, data] : conn.provides()) {
      out[id] = out[id].merge(data.clock);
    }
  }
  return out;
}

Source Broker::disconnect(Source source)
{
  if (source < 0 || _connections.size() <= static_cast<size_t>(source)) {
    return -1;
  }
  auto& conn = _connections.at(source);
  if (conn.valid()) {
    conn.disconnect();
    refreshConnections(source);
    return source;
  }
  return -1;
}

Clock Broker::clock() const
{
  return _connections.front().clock();
}

BrokerBasePtr Broker::ptr()
{
  return this->shared_from_this();
}

void Broker::setClock(const Clock& clock)
{
  _connections.front().clock() = clock;
}

bool ConnectionInfo::operator==(const ConnectionInfo& other) const
{
  return std::tie(distance, clock) == std::tie(other.distance, other.clock);
}

bool ConnectionInfo::operator<(const ConnectionInfo& other) const
{
  return std::tie(distance, clock) < std::tie(other.distance, other.clock);
}

IdConnectionInfoMap UpdateProvides(SourcesMap provides)
{
  IdConnectionInfoMap out;
  for (auto& [source, infoMap] : provides) {
    for (auto& [id, data] : infoMap) {
      ++data.distance;
    }
    out.merge(infoMap);
  }
  return out;
}

std::set<Source> Broker::connectedPorts() const
{
  std::set<Source> connected;
  for (size_t i = 1; i < _connections.size(); i++) {
    connected.insert(i);
  }
  return connected;
}

void Broker::refreshConnections(Source ignore)
{
  for (size_t i = 1; i < _connections.size(); i++) {
    if (i == static_cast<size_t>(ignore)) {
      continue;
    }
    auto& conn = _connections.at(i);
    auto data = stub();
    data.source() = static_cast<Source>(i);
    data.clock() = clock();
    data.provides() = UpdateProvides(sources(i));
    conn.refresh(data);
  }
}

Clock Broker::relay(const Data& entry, Source sender)
{
  long distance = std::numeric_limits<long>::max();
  Source shortestDistancePort = -1;
  for (const auto& [source, infoMap] : sources()) {
    if (sender > 0 && source == sender) {
      continue;
    }
    for (const auto& [id, info] : infoMap) {
      if (id == entry.id) {
        if (info.distance == 0) {
          return insert({clock().tick(id), entry});
        } else if (info.distance < distance) {
          distance = info.distance;
          shortestDistancePort = source;
        }
      }
    }
  }

  if (distance == std::numeric_limits<long>::max()) {
    return {{0, 0}};
  }

  return _connections.at(shortestDistancePort).relay(entry);
}

Connection Broker::stub()
{
  return Connection(ptr());
}

}
