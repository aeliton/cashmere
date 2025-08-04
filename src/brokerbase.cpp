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
#include "brokergrpcstub.h"

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>

namespace Cashmere
{

BrokerStub::~BrokerStub() = default;

BrokerStub::BrokerStub()
  : _url()
  , _type(Type::Invalid)
  , _memoryStub()
  , _grpcStub()
{
}

BrokerStub::BrokerStub(BrokerBasePtr broker, Type type)
  : _url()
  , _type(type)
  , _memoryStub(_type == Type::Memory ? broker : nullptr)
  , _grpcStub(_type == Type::Grpc ? broker : nullptr)
{
}

BrokerStub::BrokerStub(const std::string& url)
  : _url(url)
  , _type(Type::Grpc)
  , _memoryStub()
  , _grpcStub(std::make_shared<BrokerGrpcStub>(url))
{
}

BrokerStub::Type BrokerStub::type() const
{
  return _type;
}

void BrokerStub::reset()
{
  if (_type == Type::Memory) {
    _memoryStub.reset();
  }
}

BrokerBasePtr BrokerStub::broker() const
{
  return _type == Type::Memory ? _memoryStub.lock() : _grpcStub;
}

std::string BrokerStub::url() const
{
  return _url;
}

Connection::Connection() = default;

Connection::Connection(
  BrokerStub stub, Port port, Clock version, IdConnectionInfoMap provides
)
  : _broker(stub)
  , _cache({port, version, provides})
{
}

Connection::Connection(BrokerStub stub)
  : _broker(stub)
  , _cache()
{
}

Connection::~Connection() = default;

Port& Connection::port() const
{
  return _cache.port;
}

BrokerBasePtr Connection::broker() const
{
  return _broker.broker();
}

Clock Connection::insert(const Entry& data) const
{
  auto source = broker();
  if (!source) {
    return {};
  }
  auto clock = source->insert(data, _cache.port);
  if (!clock.valid()) {
    return {};
  }
  for (auto& [id, info] : _cache.sources) {
    info.version = info.version.merge(data.clock);
  }
  return _cache.version = clock;
}

Clock Connection::insert(const EntryList& data) const
{
  auto clock = broker()->insert(data, _cache.port);
  if (clock.valid()) {
    for (auto& [id, info] : _cache.sources) {
      info.version = info.version.merge(clock);
    }
  }
  return clock;
}

bool ConnectionData::operator==(const ConnectionData& other) const
{
  return version == other.version && sources == other.sources;
}

bool Connection::operator==(const Connection& other) const
{
  return _cache == other._cache && broker() == other.broker();
}

Clock& Connection::version(Origin origin) const
{
  if (origin == Origin::Remote) {
    _cache.version = broker()->clock();
  }
  return _cache.version;
}

EntryList Connection::entries(const Clock& clock) const
{
  return broker()->query(clock, _cache.port);
}

IdConnectionInfoMap& Connection::provides(Origin origin) const
{
  if (origin == Origin::Remote) {
    _cache.sources = broker()->provides(_cache.port);
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
    b->refresh(Connection(), _cache.port);
    _broker.reset();
  }
}

bool Connection::refresh(const Connection& data) const
{
  if (auto source = broker()) {
    return source->refresh(data, _cache.port);
  }
  return false;
}

bool Connection::active() const
{
  return broker() != nullptr;
}

void Connection::update(const Connection& data)
{
  _cache = data._cache;
}

Clock Connection::relay(const Data& entry) const
{
  return broker()->relay(entry, _cache.port);
}

BrokerBase::~BrokerBase() = default;

Clock BrokerBase::insert(const EntryList& entries, Port sender)
{
  for (const auto& entry : entries) {
    insert(entry, sender);
  }
  return clock();
}

std::ostream& operator<<(std::ostream& os, const ConnectionInfo& info)
{
  return os << "ConnectionInfo{ .distance = " << info.distance
            << ", .version = " << info.version << "}";
}

std::ostream& operator<<(std::ostream& os, const IdConnectionInfoMap& data)
{
  os << "IdConnectionInfoMap{";
  auto it = data.cbegin();
  if (it != data.cend()) {
    os << "{" << std::hex << it->first << std::dec << ", " << it->second << "}";
    for (++it; it != data.cend(); ++it) {
      os << ", {" << std::hex << it->first << std::dec << ", " << it->second
         << "}";
    }
  }
  return os << "}";
}

std::ostream& operator<<(std::ostream& os, const IdClockMap& data)
{
  os << "IdClockMap{";
  auto it = data.cbegin();
  if (it != data.cend()) {
    os << "{" << std::hex << it->first << std::dec << ", " << it->second << "}";
    for (++it; it != data.cend(); ++it) {
      os << ", {" << std::hex << it->first << std::dec << ", " << it->second
         << "}";
    }
  }
  return os << "}";
}

std::ostream& operator<<(std::ostream& os, const ConnectionData& info)
{
  return os << "ConnectionData{ .port = " << info.port
            << ".version= " << info.version << ", .provides = " << info.sources
            << "}";
}

std::ostream& operator<<(std::ostream& os, const Connection& info)
{
  return os << "Connection{" << info._cache << "}";
}

BrokerStub& Connection::stub()
{
  return _broker;
}
ConnectionData Connection::cache() const
{
  return _cache;
}

bool Connection::valid() const
{
  return _cache.port >= 0;
}

}
