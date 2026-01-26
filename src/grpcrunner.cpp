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
#include "cashmere/grpcrunner.h"
#include "cashmere/brokerstore.h"
#include "utils/grpcutils.h"

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>

namespace Cashmere
{

class GrpcRunner::Impl : public Grpc::Broker::Service
{
public:
  Impl(const std::string& hostport, BrokerBasePtr broker);

  ~Impl();

  BrokerBasePtr broker() const
  {
    return _broker.lock();
  }

  std::thread start();
  void stop();

private:
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

  BrokerBaseWeakPtr _broker;
  std::string _hostport;
  std::unique_ptr<grpc::Server> _server;
};

GrpcRunner::Impl::~Impl() {}

GrpcRunner::Impl::Impl(const std::string& hostport, BrokerBasePtr broker)
  : _broker(broker)
  , _hostport(hostport)
{
}

::grpc::Status GrpcRunner::Impl::Connect(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::ConnectionRequest* request, Grpc::ConnectionResponse* response
)
{
  auto stub = Connection(broker()->store()->getOrCreate(request->broker().url()));
  if (request->source() == 0) {
    const auto conn = broker()->connect(stub);
    response->set_source(conn.source());
  } else {
    const IdConnectionInfoMap sources =
      Utils::IdConnectionInfoMapFrom(request->sources());
    const Clock version = Utils::ClockFrom(request->clock());

    stub.source() = request->source();
    stub.clock() = version;
    stub.provides() = sources;

    Connection out = broker()->connect(stub);

    response->set_source(out.source());
    Utils::SetClock(response->mutable_clock(), out.clock());
    Utils::SetIdConnectionInfoMap(response->mutable_sources(), out.provides());
  }

  return ::grpc::Status::OK;
}

::grpc::Status GrpcRunner::Impl::Query(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::QueryRequest* request, Grpc::QueryResponse* response
)
{
  auto sender = request->sender();
  Clock clock = Utils::ClockFrom(request->clock());

  for (auto& entry : broker()->query(clock, sender)) {
    auto out = response->add_entries();
    Utils::SetEntry(out, entry);
  }

  return ::grpc::Status::OK;
}

::grpc::Status GrpcRunner::Impl::Insert(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::InsertRequest* request, Grpc::InsertResponse* response
)
{
  Source sender = request->sender();
  Entry entry = Utils::EntryFrom(request->entry());

  if (broker()->insert(entry, sender).valid()) {
    Utils::SetClock(response->mutable_clock(), broker()->clock());
    return ::grpc::Status::OK;
  }

  return ::grpc::Status::CANCELLED;
}

::grpc::Status GrpcRunner::Impl::Refresh(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RefreshRequest* request,
  [[maybe_unused]] ::google::protobuf::Empty* response
)
{
  Connection conn;
  conn.source() = request->source();
  conn.clock() = Utils::ClockFrom(request->clock());
  conn.provides() = Utils::IdConnectionInfoMapFrom(request->sources());
  broker()->refresh(conn, request->sender());
  return ::grpc::Status::OK;
}

::grpc::Status GrpcRunner::Impl::Relay(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RelayInsertRequest* request, Grpc::InsertResponse* response
)
{
  Clock clock =
    broker()->relay(Utils::DataFrom(request->entry()), request->sender());
  Utils::SetClock(response->mutable_clock(), clock);
  return ::grpc::Status::OK;
}

::grpc::Status GrpcRunner::Impl::Sources(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const ::Cashmere::Grpc::SourcesRequest* request,
  Grpc::SourcesResponse* response
)
{
  auto sources = broker()->sources(request->sender());
  Utils::SetSources(response->mutable_sources(), sources);
  return ::grpc::Status::OK;
}

std::thread GrpcRunner::Impl::start()
{
  grpc::ServerBuilder builder;
  builder.AddListeningPort(_hostport, grpc::InsecureServerCredentials());

  builder.RegisterService(this);
  _server = builder.BuildAndStart();
  return std::thread([this]() { _server->Wait(); });
}

void GrpcRunner::Impl::stop()
{
  _server->Shutdown();
}

::grpc::Status GrpcRunner::Impl::GetClock(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const ::google::protobuf::Empty*, Grpc::ClockResponse* response
)
{
  Utils::SetClock(response->mutable_clock(), broker()->clock());
  return ::grpc::Status::OK;
}

void GrpcRunner::stop()
{
  _impl->stop();
}

GrpcRunner::GrpcRunner(const std::string& hostport, BrokerBasePtr broker)
  : _impl(std::make_unique<GrpcRunner::Impl>(hostport, broker))
{
}

std::thread GrpcRunner::start()
{
  return _impl->start();
}

GrpcRunner::~GrpcRunner() = default;

}
