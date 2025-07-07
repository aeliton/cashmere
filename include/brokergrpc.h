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
#ifndef CASHMERE_BROKER_GRPC_H
#define CASHMERE_BROKER_GRPC_H

#include "broker.h"

#include <proto/cashmere.grpc.pb.h>
#include <proto/cashmere.pb.h>

namespace Cashmere
{

class BrokerGrpc;
using BrokerGrpcPtr = std::shared_ptr<BrokerGrpc>;
using BrokerGrpcWeakPtr = std::weak_ptr<BrokerGrpc>;

class BrokerGrpcStub;
using BrokerGrpcStubPtr = std::shared_ptr<BrokerGrpcStub>;
using BrokerGrpcStubWeakPtr = std::weak_ptr<BrokerGrpcStub>;

class BrokerGrpcStub : public BrokerBase,
                       public std::enable_shared_from_this<BrokerGrpcStub>
{
public:
  explicit BrokerGrpcStub(const std::string& url);
  explicit BrokerGrpcStub(std::unique_ptr<Grpc::Broker::StubInterface>&& stub);
  virtual Clock clock() const override;
  virtual IdClockMap versions() const override;
  virtual IdConnectionInfoMap provides(Port sender = 0) const override;
  virtual Clock insert(const Entry& data, Port sender = 0) override;
  virtual EntryList
  query(const Clock& from = {}, Port sender = 0) const override;

  virtual ConnectionData connect(Connection conn) override;
  virtual bool refresh(const ConnectionData& conn, Port sender) override;

private:
  std::string _url;
  std::unique_ptr<Grpc::Broker::StubInterface> _stub;
};

class BrokerGrpc : public Broker
{
public:
  BrokerGrpc(uint16_t port);

private:
  [[maybe_unused]] uint16_t _port;
};

}

#endif
