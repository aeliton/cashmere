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
#ifndef CASHMERE_BROKER_GRPC_STUB_H
#define CASHMERE_BROKER_GRPC_STUB_H

#include "cashmere/brokerbase.h"
#include <proto/cashmere.grpc.pb.h>

namespace Cashmere
{

class BrokerGrpcStub;
using BrokerGrpcStubPtr = std::shared_ptr<BrokerGrpcStub>;
using BrokerGrpcStubWeakPtr = std::weak_ptr<BrokerGrpcStub>;

class BrokerGrpcStub : public BrokerBase
{
public:
  explicit BrokerGrpcStub(const std::string& url);
  explicit BrokerGrpcStub(const std::string& hostname, uint32_t port);
  explicit BrokerGrpcStub(std::unique_ptr<Grpc::Broker::StubInterface>&& stub);
  virtual Clock clock() const override;
  virtual IdClockMap versions() const override;
  virtual SourcesMap sources(Source sender = 0) const override;
  virtual Clock insert(const Entry& data, Source sender = 0) override;
  virtual EntryList
  query(const Clock& from = {}, Source sender = 0) const override;

  virtual Connection connect(Connection conn) override;
  virtual bool refresh(const Connection& conn, Source sender) override;
  virtual BrokerStub stub() override;
  virtual Clock relay(const Data& entry, Source sender) override;

private:
  std::string _url;
  std::unique_ptr<Grpc::Broker::StubInterface> _stub;
};

}

#endif
