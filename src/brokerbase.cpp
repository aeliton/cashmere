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
  , _cache()
{
}

Connection::Connection(
  BrokerBasePtr broker, Port port, Clock version, IdConnectionInfoMap provides
)
  : _broker(broker)
  , _port(port)
  , _cache(version, provides)
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
  for (auto& [id, info] : _cache.sources) {
    info.version = info.version.merge(data.clock);
  }
  return _cache.version = clock;
}

Clock Connection::insert(const EntryList& data)
{
  auto clock = broker()->insert(data, _port);
  if (clock.valid()) {
    for (auto& [id, info] : _cache.sources) {
      info.version = info.version.merge(clock);
    }
  }
  return clock;
}

bool Connection::operator==(const Connection& other) const
{
  return _port == other._port &&
         _broker.lock().get() == other._broker.lock().get();
}

Clock& Connection::version(Origin origin) const
{
  if (origin == Origin::Remote) {
    _cache.version = broker()->clock();
  }
  return _cache.version;
}

EntryList Connection::entries(Clock clock) const
{
  return _broker.lock()->entries(clock, _port);
}

IdConnectionInfoMap& Connection::provides(Origin origin) const
{
  if (origin == Origin::Remote) {
    _cache.sources = _broker.lock()->provides(_port);
    for (auto& [id, data] : _cache.sources) {
      ++data.distance;
      _cache.version = _cache.version.merge(data.version);
    }
  }
  return _cache.sources;
}

void Connection::disconnect()
{
  if (auto b = broker()) {
    b->update({nullptr, 0, {}, {}}, _port);
    _broker.reset();
  }
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
