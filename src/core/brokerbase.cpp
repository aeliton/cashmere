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
#include "utils/urlutils.h"
#include "brokerbaseimpl.h"

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>

namespace Cashmere
{

Connection::~Connection() = default;

Connection::Connection()
  : _source(0)
  , _version()
  , _sources()
  , _broker()
{
}

Connection::Connection(
  BrokerBasePtr broker, Source source, const Clock& version,
  const IdConnectionInfoMap& sources
)
  : _source(source)
  , _version(version)
  , _sources(sources)
  , _broker(broker)
{
}

void Connection::reset()
{
  _broker.reset();
}

BrokerBasePtr Connection::broker() const
{
  return _broker.lock();
}

std::string Connection::url() const
{
  return broker() ? broker()->url() : "";
}

Source& Connection::source() const
{
  return _source;
}

Clock& Connection::clock(Origin origin) const
{
  if (origin == Origin::Remote) {
    _version = broker()->clock();
  }
  return _version;
}

IdConnectionInfoMap& Connection::provides(Origin origin) const
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

void Connection::disconnect()
{
  if (auto b = broker()) {
    b->refresh(Connection(), _source);
    reset();
  }
}

Clock Connection::insert(const Entry& data) const
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

Clock Connection::insert(const EntryList& data) const
{
  auto clock = broker()->insert(data, _source);
  if (clock.valid()) {
    for (auto& [id, info] : _sources) {
      info.clock = info.clock.merge(clock);
    }
  }
  return clock;
}

EntryList Connection::query(const Clock& clock) const
{
  return broker()->query(clock, _source);
}

Clock Connection::relay(const Data& entry) const
{
  return broker()->relay(entry, _source);
}

bool Connection::valid() const
{
  return broker() != nullptr;
}

std::string Connection::str() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

Connection& Connection::connect(Connection data)
{
  auto other = broker()->connect(data);
  _source = other._source;
  _version = other._version;
  _sources = other._sources;
  return *this;
}

bool Connection::refresh(const Connection& data) const
{
  if (auto source = broker()) {
    return source->refresh(data, _source);
  }
  return false;
}

bool Connection::operator==(const Connection& other) const
{
  return url() == other.url() &&
         _version == other._version &&
         _sources == other._sources &&
         _source == other._source;
}

BrokerBase::BrokerBase(const std::string& url)
  : _impl(std::make_unique<Impl>(url))
{
}

BrokerBase::~BrokerBase() = default;

Id BrokerBase::id() const
{
  return _impl->id;
}

std::string BrokerBase::url() const
{
  return _impl->url.url;
}

Clock BrokerBase::insert(const EntryList& entries, Source sender)
{
  for (const auto& entry : entries) {
    insert(entry, sender);
  }
  return clock();
}

bool BrokerBase::append(Amount value)
{
  return append({id(), value, {}});
}

bool BrokerBase::append(const Data& entry)
{
  return insert({clock().tick(entry.id), entry}).valid();
}

bool BrokerBase::replace(Amount value, const Clock& clock)
{
  return append({id(), value, clock});
}

bool BrokerBase::erase(Clock time)
{
  return append({id(), 0, time});
}

bool BrokerBase::contains(const Clock& time) const
{
  return entry(time).valid();
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

std::ostream& operator<<(std::ostream& os, const Connection& info)
{
  return os << "Connection{ .url: " << (info.broker() ? info.broker()->url() : "")
            << ", .source = " << info._source << ".version= " << info._version
            << ", .provides = " << info._sources << "}";
}

Data BrokerBase::entry(Clock) const
{
  return {};
}

bool BrokerBase::save(const Entry&)
{
  return {};
}

EntryList BrokerBase::entries() const
{
  return {};
}

std::string BrokerBase::location() const
{
  return _impl->url.path;
}
uint16_t BrokerBase::port() const
{
  return _impl->port;
}
std::string BrokerBase::hostname() const
{
  return _impl->hostname;
}

Connection BrokerBase::connect(const std::string& url)
{
  if (auto store = _impl->store()) {
    return connect(Connection{store->getOrCreate(url)});
  }
  return connect(Connection{});
}

Connection BrokerBase::stub()
{
  if (auto store = _impl->store()) {
    return Connection(store->getOrCreate(url()));
  }
  return Connection(ptr());
}

BrokerBasePtr BrokerBase::ptr()
{
  return this->shared_from_this();
}

BrokerBase::Impl* BrokerBase::impl()
{
  return _impl.get();
}

}
