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
#include "cashmere/brokerbase.h"
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
  , _source(0)
  , _version()
  , _sources()
  , _grpcStub()
{
}

BrokerStub::BrokerStub(BrokerBasePtr broker, Type type)
  : _url()
  , _type(type)
  , _source(0)
  , _version()
  , _sources()
  , _memoryStub(type == Type::Memory ? broker : nullptr)
  , _grpcStub(type == Type::Grpc ? broker : nullptr)
{
}

BrokerStub::BrokerStub(
  BrokerBasePtr broker, Source source, const Clock& version,
  const IdConnectionInfoMap& sources
)
  : _url()
  , _type(Type::Memory)
  , _source(source)
  , _version(version)
  , _sources(sources)
  , _memoryStub(broker)
  , _grpcStub()
{
}

BrokerStub::BrokerStub(const std::string& url)
  : _url(url)
  , _type(Type::Grpc)
  , _source(0)
  , _version()
  , _sources()
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

Source& BrokerStub::source() const
{
  return _source;
}

Clock& BrokerStub::clock(Origin origin) const
{
  if (origin == Origin::Remote) {
    _version = broker()->clock();
  }
  return _version;
}

IdConnectionInfoMap& BrokerStub::provides(Origin origin) const
{
  if (origin == Origin::Remote) {
    for (auto& [port, sources] : broker()->sources(_source)) {
      for (auto& [id, data] : sources) {
        ++data.distance;
        _version = _version.merge(data.clock);
      }
      _sources.merge(sources);
    }
  }
  return _sources;
}

void BrokerStub::disconnect()
{
  if (auto b = broker()) {
    b->refresh(BrokerStub(), _source);
    reset();
  }
}

Clock BrokerStub::insert(const Entry& data) const
{
  auto source = broker();
  if (!source) {
    return {};
  }
  auto clock = source->insert(data, _source);
  if (!clock.valid()) {
    return {};
  }
  for (auto& [id, info] : _sources) {
    info.clock = info.clock.merge(data.clock);
  }
  return _version = clock;
}

Clock BrokerStub::insert(const EntryList& data) const
{
  auto clock = broker()->insert(data, _source);
  if (clock.valid()) {
    for (auto& [id, info] : _sources) {
      info.clock = info.clock.merge(clock);
    }
  }
  return clock;
}

EntryList BrokerStub::query(const Clock& clock) const
{
  return broker()->query(clock, _source);
}

Clock BrokerStub::relay(const Data& entry) const
{
  return broker()->relay(entry, _source);
}

bool BrokerStub::valid() const
{
  return _type != BrokerStub::Type::Invalid && broker() != nullptr;
}

std::string BrokerStub::str() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

BrokerStub& BrokerStub::connect(BrokerStub data)
{
  auto other = broker()->connect(data);
  _source = other._source;
  _version = other._version;
  _sources = other._sources;
  return *this;
}

bool BrokerStub::refresh(const BrokerStub& data) const
{
  if (auto source = broker()) {
    return source->refresh(data, _source);
  }
  return false;
}

bool BrokerStub::operator==(const BrokerStub& other) const
{
  return _type == other._type && _url == other._url &&
         _memoryStub.lock() == other._memoryStub.lock() &&
         _grpcStub == other._grpcStub && _version == other._version &&
         _sources == other._sources && _source == other._source;
}

BrokerBase::~BrokerBase() = default;

Clock BrokerBase::insert(const EntryList& entries, Source sender)
{
  for (const auto& entry : entries) {
    insert(entry, sender);
  }
  return clock();
}

std::ostream& operator<<(std::ostream& os, const ConnectionInfo& info)
{
  return os << "ConnectionInfo{ .distance = " << info.distance
            << ", .version = " << info.clock << "}";
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

std::ostream& operator<<(std::ostream& os, const SourcesMap& data)
{
  os << "SourcesMap{";
  auto it = data.cbegin();
  if (it != data.cend()) {
    os << "{" << it->first << ", " << it->second << "}";
    for (++it; it != data.cend(); ++it) {
      os << ", {" << it->first << ", " << it->second << "}";
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

std::ostream& operator<<(std::ostream& os, const BrokerStub& info)
{
  return os << "BrokerStub{ .url: " << info._url
            << ", .source = " << info._source << ".version= " << info._version
            << ", .provides = " << info._sources << "}";
}

}
