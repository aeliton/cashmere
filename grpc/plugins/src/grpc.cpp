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
#include "cashmere/plugins/grpc.h"
#include "cashmere/utils/grpc.h"

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>

namespace Cashmere
{

BrokerGrpcStub::BrokerGrpcStub(
  std::unique_ptr<Grpc::Broker::StubInterface>&& stub
)
  : BrokerBase()
  , _url()
  , _stub(std::move(stub))
{
}

BrokerGrpcStub::BrokerGrpcStub(const std::string& url)
  : BrokerBase(url)
  , _stub(Grpc::Broker::NewStub(
      grpc::CreateChannel(std::format("{}:{}", hostname(), port()), grpc::InsecureChannelCredentials())
    ))
{
}

BrokerBase* BrokerGrpcStub::create(const std::string& url)
{
    return new BrokerGrpcStub(url);
}


Clock BrokerGrpcStub::clock() const
{
  ::grpc::ClientContext context;
  ::google::protobuf::Empty empty;
  Grpc::ClockResponse response;
  if (_stub->GetClock(&context, empty, &response).ok()) {
    return Utils::ClockFrom(response.clock());
  }
  return {{0, 0}};
}

IdClockMap BrokerGrpcStub::versions() const
{
  return {};
}

SourcesMap BrokerGrpcStub::sources([[maybe_unused]] Source sender) const
{
  ::grpc::ClientContext context;
  Grpc::SourcesRequest request;
  Grpc::SourcesResponse response;
  request.set_sender(sender);
  if (_stub->Sources(&context, request, &response).ok()) {
    return Utils::SourcesFrom(response.sources());
  }
  return {};
}

Clock BrokerGrpcStub::insert(const Entry& data, Source sender)
{
  Grpc::InsertRequest request;
  request.set_sender(sender);
  Utils::SetEntry(request.mutable_entry(), data);

  Grpc::InsertResponse response;
  ::grpc::ClientContext context;
  if (_stub->Insert(&context, request, &response).ok()) {
    Clock out = Utils::ClockFrom(response.clock());
    return out;
  }
  return {};
}

EntryList BrokerGrpcStub::query(const Clock& from, Source sender) const
{
  ::grpc::ClientContext context;
  Grpc::QueryRequest request;

  request.set_sender(sender);
  Utils::SetClock(request.mutable_clock(), from);

  Grpc::QueryResponse response;

  EntryList out;
  if (_stub->Query(&context, request, &response).ok()) {
    for (auto& entry : response.entries()) {
      out.push_back(Utils::EntryFrom(entry));
    }
    return out;
  }
  return {};
}

Connection BrokerGrpcStub::connect(Connection conn)
{
  ::grpc::ClientContext context;

  Grpc::ConnectionRequest request;
  request.set_source(conn.source());
  Utils::SetClock(request.mutable_clock(), conn.clock());

  request.mutable_broker()->set_url(conn.url());

  Utils::SetIdConnectionInfoMap(request.mutable_sources(), conn.provides());

  Grpc::ConnectionResponse response;

  auto status = _stub->Connect(&context, request, &response);
  if (status.ok()) {
    Clock clock = Utils::ClockFrom(response.clock());
    Connection data = stub();
    data.source() = response.source();
    data.clock() = clock;
    return data;
  }
  return Connection{};
}

bool BrokerGrpcStub::refresh(const Connection& conn, Source sender)
{
  Grpc::RefreshRequest request;
  request.set_sender(sender);
  request.set_source(conn.source());

  Utils::SetClock(request.mutable_clock(), conn.clock());
  Utils::SetIdConnectionInfoMap(request.mutable_sources(), conn.provides());

  ::google::protobuf::Empty response;
  ::grpc::ClientContext context;
  if (_stub->Refresh(&context, request, &response).ok()) {
    return true;
  }

  return {};
}

Clock BrokerGrpcStub::relay(const Data& entry, Source sender)
{
  ::grpc::ClientContext context;
  Grpc::RelayInsertRequest request;
  Grpc::InsertResponse response;
  request.set_sender(sender);
  Utils::SetData(request.mutable_entry(), entry);
  const auto status = _stub->Relay(&context, request, &response);
  if (status.ok()) {
    return Utils::ClockFrom(response.clock());
  }
  return {{0, 0}};
}

#ifdef CASHMERE_GRPC_BUILD_PLUGIN
extern "C" CASHMERE_EXPORT
#endif
Cashmere::BrokerBase* create(const std::string& url)
{
  return new Cashmere::BrokerGrpcStub(url);
}
}
