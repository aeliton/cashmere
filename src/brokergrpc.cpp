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
#include "brokergrpc.h"

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

Id BrokerGrpcStub::id() const
{
  return {};
}

Clock BrokerGrpcStub::clock() const
{
  return {};
}

IdClockMap BrokerGrpcStub::versions() const
{
  return {};
}

void BrokerGrpcStub::setClock([[maybe_unused]] const Clock& clock) {}
IdConnectionInfoMap BrokerGrpcStub::provides([[maybe_unused]] Port sender) const
{
  return {};
}

Clock BrokerGrpcStub::insert(
  [[maybe_unused]] const Entry& data, [[maybe_unused]] Port sender
)
{
  return {};
}

EntryList
BrokerGrpcStub::query(const Clock& from, [[maybe_unused]] Port sender) const
{
  ::grpc::ClientContext context;
  Grpc::Clock clock;
  for (auto& [id, count] : from) {
    (*clock.mutable_data())[id] = count;
  }
  Grpc::EntryList entries;

  EntryList out;
  if (_stub->Query(&context, clock, &entries).ok()) {
    for (auto& e : entries.data()) {
      Clock clock;
      Data data;
      for (auto& [id, count] : e.clock().data()) {
        clock[id] = count;
      }
      data.id = e.data().id();
      data.value = e.data().value();
      for (auto& [id, count] : e.data().alters().data()) {
        data.alters[id] = count;
      }
      out.push_back({clock, data});
    }
    return out;
  }
  return {};
}

ConnectionData BrokerGrpcStub::connect(Connection conn)
{
  ::grpc::ClientContext context;
  Grpc::Connection request;
  request.set_port(conn.port());
  Grpc::Connection response;

  if (_stub->Connect(&context, request, &response).ok()) {
    Clock clock;
    for (auto& [id, count] : response.version().data()) {
      clock[id] = count;
    }
    return ConnectionData{response.port(), clock, {}};
  }
  return {};
}

bool BrokerGrpcStub::refresh(
  [[maybe_unused]] const ConnectionData& conn, [[maybe_unused]] Port sender
)
{
  return {};
}

Port BrokerGrpcStub::disconnect([[maybe_unused]] Port port)
{
  return {};
}

std::set<Port> BrokerGrpcStub::connectedPorts() const
{
  return {};
}

BrokerBasePtr BrokerGrpcStub::ptr()
{
  return shared_from_this();
}

BrokerGrpc::BrokerGrpc(uint16_t port)
  : Broker()
  , _port(port)
{
}

}
