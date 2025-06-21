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

Connection::Connection()
  : _broker(std::shared_ptr<BrokerI>(nullptr))
  , _port(0)
  , _version({})
  , _provides({})
{
}

Connection::Connection(
  BrokerIPtr broker, Port port, Clock version, IdConnectionInfoMap provides
)
  : _broker(broker)
  , _port(port)
  , _version(version)
  , _provides(provides)
{
}

Port Connection::port() const
{
  return _port;
}

BrokerIPtr Connection::broker() const
{
  return _broker.lock();
}

Clock Connection::insert(const Entry& data)
{
  auto source = _broker.lock();
  if (!source) {
    return {};
  }
  auto clock = source->insert(data, _port);
  if (!clock.valid()) {
    return {};
  }
  for (auto& [id, info] : _provides) {
    info.version = info.version.merge(data.clock);
  }
  return _version = clock;
}

Clock Connection::insert(const EntryList& data)
{
  auto clock = broker()->insert(data, _port);
  if (clock.valid()) {
    for (auto& [id, info] : _provides) {
      info.version = info.version.merge(clock);
    }
  }
  return clock;
}

void Connection::updateProvides()
{
  _provides = _broker.lock()->provides(_port);
  for (auto& [id, data] : _provides) {
    ++data.distance;
  }
}

bool Connection::operator==(const Connection& other) const
{
  return _port == other._port &&
         _broker.lock().get() == other._broker.lock().get();
}
Clock Connection::merge(const Clock& clock)
{
  return _version = _version.merge(clock);
}

Clock Connection::version() const
{
  return _version;
}

EntryList Connection::entries(Clock clock) const
{
  return _broker.lock()->entries(clock, _port);
}

IdConnectionInfoMap Connection::provides() const
{
  return _provides;
}

void Connection::setProvides(Id id, int64_t distance)
{
  _provides[id].distance = distance;
}

void Connection::setProvides(Id id, Clock version)
{
  _provides[id].version = version;
}

bool Connection::provides(Id id) const
{
  return _provides.find(id) != _provides.cend();
}

void Connection::reset()
{
  _broker.reset();
}

void Connection::setVersion(Clock clock)
{
  _version = clock;
}

BrokerI::~BrokerI() = default;

Clock BrokerI::insert(const EntryList& entries, Port sender)
{
  for (const auto& entry : entries) {
    insert(entry, sender);
  }
  return clock();
}

Broker::Broker()
{
  _connections.push_back({});
}

Broker::~Broker() = default;

Connection Broker::connect(Connection conn)
{
  Port port = _connections.size();
  _connections.push_back(conn);
  return {ptr(), port, clock(), provides(conn.port())};
}

Port Broker::connect(BrokerIPtr remote)
{
  if (!remote) {
    return -1;
  }
  const Port port = _connections.size();
  _connections.push_back(remote->connect({ptr(), port, clock(), provides()}));

  auto thisEntries = entries(_connections.back().version(), port);
  auto brokerEntries = _connections.back().entries(clock());

  if (brokerEntries.size() > 0) {
    BrokerI::insert(brokerEntries, port);
  }
  if (thisEntries.size() > 0) {
    _connections.back().insert(thisEntries);
  }

  return port;
}

Clock Broker::insert(const Entry& data, Port port)
{
  if (port < 0 || port >= _connections.size()) {
    return Clock();
  }

  setClock(clock().merge(data.clock));

  auto& ctx = _connections[port];
  if (auto j = ctx.broker()) {
    ctx.updateProvides();
  }
  ctx.setProvides(data.entry.id, clock());

  for (size_t i = 0; i < _connections.size(); ++i) {
    if (i == port) {
      continue;
    }
    auto& ctx = _connections[i];
    if (ctx.provides(data.entry.id)) {
      continue;
    }
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
    auto& ctx = _connections[i];
    if (ctx.broker()) {
      for (auto& [id, data] : ctx.provides()) {
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

EntryList Broker::entries(const Clock& from, Port ignore) const
{
  if (id() > 0) {
    return entries(from);
  }
  for (size_t i = 1; i < _connections.size(); i++) {
    auto context = _connections[i];
    if (i == ignore) {
      continue;
    }
    if (auto broker = context.broker()) {
      return context.entries(from);
    }
  }
  return {};
}

IdClockMap Broker::versions() const
{
  IdClockMap out;
  for (auto& context : _connections) {
    for (auto& [id, data] : context.provides()) {
      out[id] = data.version;
    }
  }
  return out;
}

Port Broker::disconnect(Port port)
{
  if (port < 0 || _connections.size() <= port) {
    return -1;
  }
  auto& context = _connections.at(port);
  if (context.broker()) {
    context.reset();
    return port;
  }
  return -1;
}

Clock Broker::clock() const
{
  return _connections.front().version();
}

EntryList Broker::entries(const Clock& from) const
{
  return entries(from, -1);
}

BrokerIPtr Broker::ptr()
{
  return this->shared_from_this();
}

void Broker::setClock(const Clock& clock)

{
  _connections.front().setVersion(clock);
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

}
