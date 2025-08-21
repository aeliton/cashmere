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
#include "utils/grpcutils.h"

#include <google/protobuf/empty.pb.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

namespace Cashmere::Utils
{

Clock ClockFrom(const ::google::protobuf::Map<uint64_t, uint64_t>& version)
{
  Clock out;
  for (const auto& [id, count] : version) {
    out[id] = count;
  }
  return out;
}

Data DataFrom(const Grpc::Data& data)
{
  Data out;
  out.id = data.id();
  out.value = data.value();
  out.alters = ClockFrom(data.alters());
  return out;
}

Entry EntryFrom(const Grpc::Entry& entry)
{
  Entry out;
  out.clock = ClockFrom(entry.clock());
  out.entry = DataFrom(entry.data());
  return out;
}

IdConnectionInfoMap IdConnectionInfoMapFrom(
  const google::protobuf::Map<uint64_t, Grpc::ConnectionInfo>& sources
)
{
  IdConnectionInfoMap out;
  for (const auto& [id, info] : sources) {
    out[id].distance = info.distance();
    out[id].version = ClockFrom(info.version());
  }
  return out;
}

void SetClock(
  ::google::protobuf::Map<uint64_t, uint64_t>* version, const Clock& data
)
{
  for (const auto& [id, count] : data) {
    (*version)[id] = count;
  }
}

void SetData(Grpc::Data* entry, const Data& data)
{
  entry->set_id(data.id);
  entry->set_value(data.value);
  SetClock(entry->mutable_alters(), data.alters);
}

void SetEntry(Grpc::Entry* entry, const Entry& data)
{
  SetClock(entry->mutable_clock(), data.clock);
  SetData(entry->mutable_data(), data.entry);
}

void SetConnectionInfo(Grpc::ConnectionInfo& info, const ConnectionInfo& data)
{
  info.set_distance(data.distance);
  SetClock(info.mutable_version(), data.version);
}

void SetConnectionInfo(Grpc::ConnectionInfo* info, const ConnectionInfo& data)
{
  SetConnectionInfo(*info, data);
}

void SetIdConnectionInfoMap(
  google::protobuf::Map<uint64_t, Grpc::ConnectionInfo>* sources,
  const IdConnectionInfoMap& data
)
{
  for (const auto& [id, info] : data) {
    SetConnectionInfo((*sources)[id], info);
  }
}

BrokerStub BrokerStubFrom(const Grpc::BrokerData& data)
{
  return BrokerStub(data.url());
}

}
