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
#ifndef CASHMERE_UTILS_GRPCUTILS_H
#define CASHMERE_UTILS_GRPCUTILS_H

#include "cashmere/brokerbase.h"
#include <google/protobuf/map.h>

namespace Cashmere::Grpc
{
class BrokerData;
class ConnectionInfo;
class Data;
class Entry;
class IdConnectionInfoMap;
}

namespace Cashmere::Utils
{

Clock ClockFrom(const ::google::protobuf::Map<uint64_t, uint64_t>& version);
Data DataFrom(const Grpc::Data& data);
Entry EntryFrom(const Grpc::Entry& entry);
IdConnectionInfoMap IdConnectionInfoMapFrom(
  const google::protobuf::Map<uint64_t, Grpc::ConnectionInfo>& sources
);
SourcesMap SourcesFrom(
  const google::protobuf::Map<uint32_t, Grpc::IdConnectionInfoMap>& sources
);

void SetClock(
  google::protobuf::Map<uint64_t, uint64_t>* version, const Clock& data
);
void SetData(Grpc::Data* entry, const Data& data);
void SetEntry(Grpc::Entry* proto, const Entry& entry);
void SetConnectionInfo(Grpc::ConnectionInfo& info, const ConnectionInfo& data);
void SetConnectionInfo(Grpc::ConnectionInfo* info, const ConnectionInfo& data);
void SetIdConnectionInfoMap(
  google::protobuf::Map<uint64_t, Grpc::ConnectionInfo>* sources,
  const IdConnectionInfoMap& data
);
void SetSources(
  google::protobuf::Map<uint32_t, Grpc::IdConnectionInfoMap>* proto,
  const SourcesMap sources
);

}

#endif
