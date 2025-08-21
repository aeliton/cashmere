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
#include "brokergrpcstub.h"
#include "utils/grpcutils.h"

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;

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
  : BrokerBase()
  , _url(url)
  , _stub(Grpc::Broker::NewStub(
      grpc::CreateChannel(_url, grpc::InsecureChannelCredentials())
    ))
{
}

BrokerGrpcStub::BrokerGrpcStub(const std::string& hostname, uint16_t port)
  : BrokerGrpcStub(hostname + ":" + std::to_string(port))
{
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
    Clock out = Utils::ClockFrom(response.version());
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
  lg::info("Broker grpc stub connect called with: {}", conn.str());
  ::grpc::ClientContext context;

  Grpc::ConnectionRequest request;
  request.set_source(conn.source());
  Utils::SetClock(request.mutable_version(), conn.version());

  request.mutable_broker()->set_url(conn.stub().url());

  Utils::SetIdConnectionInfoMap(request.mutable_sources(), conn.provides());

  Grpc::ConnectionResponse response;

  auto status = _stub->Connect(&context, request, &response);
  if (status.ok()) {
    Clock clock = Utils::ClockFrom(response.version());
    return Connection{BrokerStub{_url}, response.source(), clock, {}};
  } else {
    lg::error(
      "BrokerGrpcStub: error {} connecting to {}",
      static_cast<int>(status.error_code()), conn.stub().url()
    );
  }
  return Connection();
}

bool BrokerGrpcStub::refresh(const Connection& conn, Source sender)
{
  Grpc::RefreshRequest request;
  request.set_sender(sender);
  request.set_source(conn.source());

  Utils::SetClock(request.mutable_version(), conn.version());
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
    return Utils::ClockFrom(response.version());
  } else {
    lg::error(
      "BrokerGrpcStub: failed relaying message.. status {}",
      static_cast<int>(status.error_code())
    );
  }
  return {{0, 0}};
}

BrokerStub BrokerGrpcStub::stub()
{
  return BrokerStub(_url);
}

}
