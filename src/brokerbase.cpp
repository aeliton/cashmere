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
#include "brokerbase.h"

namespace Cashmere
{

Connection::Connection()
  : _broker(std::shared_ptr<BrokerBase>(nullptr))
  , _port(0)
  , _version({})
  , _provides({})
{
}

Connection::Connection(
  BrokerBasePtr broker, Port port, Clock version, IdConnectionInfoMap provides
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

BrokerBasePtr Connection::broker() const
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

void Connection::sync()
{
  _provides = _broker.lock()->provides(_port);
  for (auto& [id, data] : _provides) {
    ++data.distance;
    _version = _version.merge(data.version);
  }
}

bool Connection::operator==(const Connection& other) const
{
  return _port == other._port &&
         _broker.lock().get() == other._broker.lock().get();
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

void Connection::setProvides(Id id, Clock version)
{
  _provides[id].version = _provides[id].version.merge(version);
}

bool Connection::provides(Id id) const
{
  return _provides.find(id) != _provides.cend();
}

void Connection::disconnect()
{
  if (auto b = broker()) {
    b->update({nullptr, 0, {}, {}}, _port);
    _broker.reset();
  }
}

void Connection::setVersion(Clock clock)
{
  _version = clock;
}

bool Connection::reconnect(Connection conn) const
{
  if (auto source = broker()) {
    return source->update(conn, _port);
  }
  return false;
}

BrokerBase::~BrokerBase() = default;

Clock BrokerBase::insert(const EntryList& entries, Port sender)
{
  for (const auto& entry : entries) {
    insert(entry, sender);
  }
  return clock();
}

}
