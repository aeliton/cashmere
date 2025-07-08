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

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>

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

Clock BrokerGrpcStub::clock() const
{
  return {};
}

IdClockMap BrokerGrpcStub::versions() const
{
  return {};
}

IdConnectionInfoMap BrokerGrpcStub::provides([[maybe_unused]] Port sender) const
{
  return {};
}

Clock BrokerGrpcStub::insert(const Entry& data, Port sender)
{
  Grpc::InsertRequest request;
  request.set_sender(sender);
  auto entry = request.mutable_entry();
  for (const auto& [id, count] : data.clock) {
    (*entry->mutable_clock())[id] = count;
  }
  entry->mutable_data()->set_id(data.entry.id);
  entry->mutable_data()->set_value(data.entry.value);

  auto alters = entry->mutable_data()->mutable_alters();
  for (const auto& [id, count] : data.entry.alters) {
    (*alters)[id] = count;
  }

  Grpc::InsertResponse response;
  ::grpc::ClientContext context;
  if (_stub->Insert(&context, request, &response).ok()) {
    Clock out;
    for (const auto& [id, count] : response.version()) {
      out[id] = count;
    }
    return out;
  }
  return {};
}

EntryList BrokerGrpcStub::query(const Clock& from, Port sender) const
{
  ::grpc::ClientContext context;
  Grpc::QueryRequest request;
  request.set_sender(sender);
  for (auto& [id, count] : from) {
    (*request.mutable_clock())[id] = count;
  }
  Grpc::QueryResponse response;

  EntryList out;
  if (_stub->Query(&context, request, &response).ok()) {
    for (auto& e : response.entries()) {
      Clock clock;
      Data data;
      for (auto& [id, count] : e.clock()) {
        clock[id] = count;
      }
      data.id = e.data().id();
      data.value = e.data().value();
      for (auto& [id, count] : e.data().alters()) {
        data.alters[id] = count;
      }
      out.push_back({clock, data});
    }
    return out;
  }
  return {};
}

ConnectionData BrokerGrpcStub::connect(ConnectionData conn)
{
  ::grpc::ClientContext context;
  Grpc::ConnectionRequest request;
  request.set_port(conn.port);
  request.mutable_broker()->set_url(conn.broker.url());
  Grpc::ConnectionResponse response;

  auto status = _stub->Connect(&context, request, &response);
  if (status.ok()) {
    Clock clock;
    for (auto& [id, count] : response.version()) {
      clock[id] = count;
    }
    return ConnectionData{BrokerStub{}, response.port(), clock, {}};
  } else {
    std::cerr << "BrokerGrpcStub: error connecting to " << conn.broker.url()
              << "[" << status.error_code() << "]" << std::endl;
  }
  return ConnectionData();
}

bool BrokerGrpcStub::refresh(const ConnectionData& conn, Port sender)
{

  Grpc::RefreshRequest request;
  request.set_sender(sender);
  request.set_port(conn.port);
  for (const auto& [id, count] : conn.version) {
    (*request.mutable_version())[id] = count;
  }

  ::google::protobuf::Empty response;
  ::grpc::ClientContext context;
  if (_stub->Refresh(&context, request, &response).ok()) {
    return true;
  }

  return {};
}

BrokerStub BrokerGrpcStub::stub()
{
  return BrokerStub(_url);
}

}
