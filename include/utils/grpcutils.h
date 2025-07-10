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

#include "brokerbase.h"
#include <proto/cashmere.pb.h>

namespace Cashmere::Utils
{

Clock ClockFrom(const ::google::protobuf::Map<uint64_t, uint64_t>& version);
Data DataFrom(const Grpc::Data& data);
Entry EntryFrom(const Grpc::Entry& entry);
IdConnectionInfoMap
SourcesFrom(const google::protobuf::Map<int32_t, Grpc::ConnectionInfo>& sources
);
BrokerStub BrokerStubFrom(const Grpc::BrokerData& data);

void SetClock(
  google::protobuf::Map<uint64_t, uint64_t>* version, const Clock& data
);
void SetEntry(Grpc::Entry* proto, const Entry& entry);
void SetConnectionInfo(Grpc::ConnectionInfo& info, const ConnectionInfo& data);
void SetConnectionInfo(Grpc::ConnectionInfo* info, const ConnectionInfo& data);
void SetSources(
  google::protobuf::Map<int32_t, Grpc::ConnectionInfo>* sources,
  const IdConnectionInfoMap& data
);

}

#endif
