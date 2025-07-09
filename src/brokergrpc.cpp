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
  const ::Cashmere::Grpc::ConnectionRequest* request,
  ::Cashmere::Grpc::ConnectionResponse* response
)
{
  Clock version;
  for (const auto& [id, count] : request->version()) {
    version[id] = count;
  }

  ConnectionData conn{
    BrokerStub(request->broker().url()), request->port(), version, {}
  };

  ConnectionData out = connect(conn);

  response->set_port(out.port);
  for (const auto& [id, count] : out.version) {
    (*response->mutable_version())[id] = count;
  }

  return ::grpc::Status::OK;
}
::grpc::Status BrokerGrpc::Query(
  [[maybe_unused]] ::grpc::ServerContext* context,
  [[maybe_unused]] const ::Cashmere::Grpc::QueryRequest* request,
  [[maybe_unused]] ::Cashmere::Grpc::QueryResponse* response
)
{
  return ::grpc::Status::OK;
}
::grpc::Status BrokerGrpc::Insert(
  [[maybe_unused]] ::grpc::ServerContext* context,
  [[maybe_unused]] const ::Cashmere::Grpc::InsertRequest* request,
  [[maybe_unused]] ::Cashmere::Grpc::InsertResponse* response
)
{
  return ::grpc::Status::OK;
}
::grpc::Status BrokerGrpc::Refresh(
  [[maybe_unused]] ::grpc::ServerContext* context,
  [[maybe_unused]] const ::Cashmere::Grpc::RefreshRequest* request,
  [[maybe_unused]] ::google::protobuf::Empty* response
)
{
  return ::grpc::Status::OK;
}
void BrokerGrpc::start()
{
  std::stringstream ss;
  ss << "0.0.0.0:" << _port;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(ss.str(), grpc::InsecureServerCredentials());

  builder.RegisterService(this);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

BrokerStub BrokerGrpc::stub()
{
  return BrokerStub(_hostname + ":" + std::to_string(_port));
}

}
