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
#include "cashmere/brokergrpc.h"
#include "utils/grpcutils.h"

#include <google/protobuf/empty.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace lg = spdlog;

namespace Cashmere
{

class BrokerGrpc::Impl : public Grpc::Broker::Service
{
public:
  Impl();

  ~Impl();

  void setBroker(BrokerPtr broker);

  BrokerPtr broker() const
  {
    return _broker.lock();
  }

  std::thread start(const std::string& url);
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

  std::weak_ptr<Broker> _broker;
  std::unique_ptr<grpc::Server> _server;
};

void BrokerGrpc::Impl::setBroker(BrokerPtr broker)
{
  _broker = broker;
}

BrokerGrpc::Impl::~Impl() {}

BrokerGrpc::Impl::Impl()
  : _broker()
{
}

::grpc::Status BrokerGrpc::Impl::Connect(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::ConnectionRequest* request, Grpc::ConnectionResponse* response
)
{
  auto stub = Utils::BrokerStubFrom(request->broker());
  if (request->source() == 0) {
    const auto conn = broker()->connect(stub);
    response->set_source(conn.source());
  } else {
    const IdConnectionInfoMap sources =
      Utils::IdConnectionInfoMapFrom(request->sources());
    const Clock version = Utils::ClockFrom(request->version());

    lg::info(
      "BrokerGrpc: Connection request from: {}, source: {}, version: {}",
      request->broker().url(), request->source(), version.str()
    );

    Connection conn{stub, request->source(), version, sources};

    Connection out = broker()->connect(conn);

    response->set_source(out.source());
    Utils::SetClock(response->mutable_version(), out.version());
    Utils::SetIdConnectionInfoMap(response->mutable_sources(), out.provides());
  }

  return ::grpc::Status::OK;
}

::grpc::Status BrokerGrpc::Impl::Query(
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

::grpc::Status BrokerGrpc::Impl::Insert(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::InsertRequest* request, Grpc::InsertResponse* response
)
{
  Source sender = request->sender();
  Entry entry = Utils::EntryFrom(request->entry());

  lg::info(
    "BrokerGrpc: Insert request from sender: {}, entry: {}", sender, entry.str()
  );

  if (broker()->insert(entry, sender).valid()) {
    Utils::SetClock(response->mutable_version(), broker()->clock());
    return ::grpc::Status::OK;
  }

  return ::grpc::Status::CANCELLED;
}

::grpc::Status BrokerGrpc::Impl::Refresh(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RefreshRequest* request,
  [[maybe_unused]] ::google::protobuf::Empty* response
)
{
  Connection conn;
  conn.source() = request->source();
  conn.version() = Utils::ClockFrom(request->version());
  conn.provides() = Utils::IdConnectionInfoMapFrom(request->sources());
  broker()->refresh(conn, request->sender());
  lg::info("BrokerGrpc: Refresh called!");
  return ::grpc::Status::OK;
}

::grpc::Status BrokerGrpc::Impl::Relay(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RelayInsertRequest* request, Grpc::InsertResponse* response
)
{
  lg::info(
    "BrokerGrpc: Relay message received: {}, sender: {}",
    Utils::DataFrom(request->entry()).str(), request->sender()
  );
  Clock clock =
    broker()->relay(Utils::DataFrom(request->entry()), request->sender());
  Utils::SetClock(response->mutable_version(), clock);
  return ::grpc::Status::OK;
}

::grpc::Status BrokerGrpc::Impl::Sources(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const ::Cashmere::Grpc::SourcesRequest* request,
  Grpc::SourcesResponse* response
)
{
  auto sources = broker()->sources(request->sender());
  Utils::SetSources(response->mutable_sources(), sources);
  return ::grpc::Status::OK;
}

std::thread BrokerGrpc::Impl::start(const std::string& url)
{
  grpc::ServerBuilder builder;
  builder.AddListeningPort(url, grpc::InsecureServerCredentials());

  builder.RegisterService(this);
  _server = builder.BuildAndStart();
  return std::thread([this]() { _server->Wait(); });
}

void BrokerGrpc::Impl::stop()
{
  _server->Shutdown();
}

::grpc::Status BrokerGrpc::Impl::GetClock(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const ::google::protobuf::Empty*, Grpc::ClockResponse* response
)
{
  Utils::SetClock(response->mutable_clock(), broker()->clock());
  return ::grpc::Status::OK;
}

void BrokerGrpc::stop()
{
  _impl->stop();
}

BrokerGrpc::BrokerGrpc(const std::string& hostname, uint16_t port)
  : Broker()
  , _hostname(hostname)
  , _port(port)
  , _impl(std::make_unique<BrokerGrpc::Impl>())
{
}

BrokerStub BrokerGrpc::stub()
{
  return BrokerStub(_hostname + ":" + std::to_string(_port));
}

std::thread BrokerGrpc::start()
{
  _impl->setBroker(shared_from_this());

  lg::info("BrokerGrpc: running on port: {}", _port);
  std::stringstream ss;
  ss << "0.0.0.0:" << _port;
  return _impl->start(ss.str());
}

BrokerGrpc::~BrokerGrpc() = default;

}
