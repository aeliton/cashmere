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
#ifndef CASHMERE_BROKER_GRPC_RUNNER_H
#define CASHMERE_BROKER_GRPC_RUNNER_H

#include "cashmere/brokerwrapper.h"
#include "cashmere/cashmere.h"

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>
#include <memory>
#include <thread>

namespace Cashmere
{

class GrpcRunner;
using GrpcRunnerPtr = std::shared_ptr<GrpcRunner>;
using GrpcRunnerWeakPtr = std::weak_ptr<GrpcRunner>;

class CASHMERE_EXPORT GrpcRunner : public WrapperBase, public Grpc::Broker::Service
{
public:
  GrpcRunner(const std::string& url);

  static WrapperBasePtr create(const std::string& url);

  std::thread start(BrokerBasePtr broker) override;
  void stop() override;

  BrokerBasePtr broker();

private:
  BrokerBaseWeakPtr _broker;
  ::grpc::Status Connect(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::ConnectionRequest* request,
    ::Cashmere::Grpc::ConnectionResponse* response
  ) override;
  ::grpc::Status Query(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::QueryRequest* request,
    ::Cashmere::Grpc::QueryResponse* response
  ) override;
  ::grpc::Status Insert(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::InsertRequest* request,
    ::Cashmere::Grpc::InsertResponse* response
  ) override;
  ::grpc::Status Refresh(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::RefreshRequest* request,
    ::google::protobuf::Empty* response
  ) override;
  ::grpc::Status Relay(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::RelayInsertRequest* request,
    ::Cashmere::Grpc::InsertResponse* response
  ) override;
  ::grpc::Status GetClock(
    ::grpc::ServerContext* context, const ::google::protobuf::Empty* request,
    ::Cashmere::Grpc::ClockResponse* response
  ) override;
  ::grpc::Status Sources(
    ::grpc::ServerContext* context,
    const ::Cashmere::Grpc::SourcesRequest* request,
    ::Cashmere::Grpc::SourcesResponse* response
  ) override;

  std::unique_ptr<grpc::Server> _server;
};

}

#endif
