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
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>
#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>
#include <thread>

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
  if (request->port() == 0) {
    const auto conn = broker()->connect(stub);
    response->set_port(conn.port());
  } else {
    const IdConnectionInfoMap sources = Utils::SourcesFrom(request->sources());
    const Clock version = Utils::ClockFrom(request->version());

    std::cout << "Connection request from: [" << request->broker().url()
              << "] with port: " << request->port() << ", version: " << version
              << std::endl
              << ", sources: " << sources << std::endl;

    Connection conn{stub, request->port(), version, sources};

    Connection out = broker()->connect(conn);

    response->set_port(out.port());
    Utils::SetClock(response->mutable_version(), out.version());
    Utils::SetSources(response->mutable_sources(), out.provides());
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
  Port sender = request->sender();
  Entry entry = Utils::EntryFrom(request->entry());

  std::cout << "Insert request from sender: [" << sender << "] entry: " << entry
            << std::endl;

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
  conn.port() = request->port();
  conn.version() = Utils::ClockFrom(request->version());
  conn.provides() = Utils::SourcesFrom(request->sources());
  broker()->refresh(conn, request->sender());
  std::cout << "Refresh called!" << std::endl;
  return ::grpc::Status::OK;
}

::grpc::Status BrokerGrpc::Impl::Relay(
  [[maybe_unused]] ::grpc::ServerContext* context,
  const Grpc::RelayInsertRequest* request, Grpc::InsertResponse* response
)
{
  std::cout << "Relay message received: " << Utils::DataFrom(request->entry())
            << " and sender: " << request->sender() << std::endl;
  Clock clock =
    broker()->relay(Utils::DataFrom(request->entry()), request->sender());
  Utils::SetClock(response->mutable_version(), clock);
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

  std::cout << "BrokerGrpc running on port: " << _port << std::endl;
  std::stringstream ss;
  ss << "0.0.0.0:" << _port;
  return _impl->start(ss.str());
}

BrokerGrpc::~BrokerGrpc() = default;

}
