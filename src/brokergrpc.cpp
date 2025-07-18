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
#include "utils/grpcutils.h"

#include <google/protobuf/empty.pb.h>
#include <grpcpp/create_channel.h>
#include <proto/cashmere.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

namespace Cashmere
{

BrokerGrpc::BrokerGrpc(const std::string& hostname, uint16_t port)
  : Broker()
  , _hostname(hostname)
  , _port(port)
{
}

::grpc::Status BrokerGrpc::Connect(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::ConnectionRequest* request, Grpc::ConnectionResponse* response
)
{
  auto stub = Utils::BrokerStubFrom(request->broker());
  if (request->port() == 0) {
    const auto conn = connect(stub);
    response->set_port(conn.port());
  } else {
    const IdConnectionInfoMap sources = Utils::SourcesFrom(request->sources());
    const Clock version = Utils::ClockFrom(request->version());

    std::cout << "Connection request from: [" << request->broker().url()
              << "] with port: " << request->port() << ", version: " << version
              << std::endl
              << ", sources: " << sources << std::endl;

    Connection conn{stub, request->port(), version, sources};

    Connection out = connect(conn);

    response->set_port(out.port());
    Utils::SetClock(response->mutable_version(), out.version());
    Utils::SetSources(response->mutable_sources(), out.provides());
  }

  return ::grpc::Status::OK;
}
::grpc::Status BrokerGrpc::Query(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::QueryRequest* request, Grpc::QueryResponse* response
)
{
  auto sender = request->sender();
  Clock clock = Utils::ClockFrom(request->clock());

  for (auto& entry : query(clock, sender)) {
    auto out = response->add_entries();
    Utils::SetEntry(out, entry);
  }

  return ::grpc::Status::OK;
}
::grpc::Status BrokerGrpc::Insert(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::InsertRequest* request, Grpc::InsertResponse* response
)
{
  Port sender = request->sender();
  Entry entry = Utils::EntryFrom(request->entry());

  std::cout << "Insert request from sender: [" << sender << "] entry: " << entry
            << std::endl;

  if (insert(entry, sender).valid()) {
    Utils::SetClock(response->mutable_version(), clock());
    return ::grpc::Status::OK;
  }

  return ::grpc::Status::CANCELLED;
}
::grpc::Status BrokerGrpc::Refresh(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RefreshRequest* request,
  [[maybe_unused]] ::google::protobuf::Empty* response
)
{
  Connection conn;
  conn.port() = request->port();
  conn.version() = Utils::ClockFrom(request->version());
  conn.provides() = Utils::SourcesFrom(request->sources());
  refresh(conn, request->sender());
  std::cout << "Refresh called!" << std::endl;
  return ::grpc::Status::OK;
}

::grpc::Status BrokerGrpc::Relay(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RelayInsertRequest* request, Grpc::InsertResponse* response
)
{
  std::cout << "Relay message received: " << Utils::DataFrom(request->entry())
            << " and sender: " << request->sender() << std::endl;
  Clock clock = relay(Utils::DataFrom(request->entry()), request->sender());
  Utils::SetClock(response->mutable_version(), clock);
  return ::grpc::Status::OK;
}

std::unique_ptr<grpc::Server> BrokerGrpc::start()
{
  std::cout << "BrokerGrpc running on port: " << _port << std::endl;
  std::stringstream ss;
  ss << "0.0.0.0:" << _port;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(ss.str(), grpc::InsecureServerCredentials());

  builder.RegisterService(this);
  return builder.BuildAndStart();
}

::grpc::Status BrokerGrpc::GetClock(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const ::google::protobuf::Empty*, Grpc::ClockResponse* response
)
{
  Utils::SetClock(response->mutable_clock(), clock());
  return ::grpc::Status::OK;
}

BrokerStub BrokerGrpc::stub()
{
  return BrokerStub(_hostname + ":" + std::to_string(_port));
}

}
