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
#ifndef CASHMERE_BROKER_GRPC_CLIENT_H
#define CASHMERE_BROKER_GRPC_CLIENT_H

#include "cashmere/brokerbase.h"
#include <cstdint>
#include <string>

namespace Cashmere
{

class BrokerGrpcStub;

class CASHMERE_EXPORT BrokerGrpcClient : public BrokerBase
{
public:
  BrokerGrpcClient(const std::string& hostname, uint16_t port);

  ~BrokerGrpcClient();
  virtual Clock clock() const override;
  virtual IdClockMap versions() const override;
  virtual SourcesMap sources(Port sender = 0) const override;
  virtual Clock insert(const Entry& data, Port sender = 0) override;
  virtual EntryList
  query(const Clock& from = {}, Port sender = 0) const override;

  virtual Connection connect(Connection conn) override;
  virtual bool refresh(const Connection& conn, Port sender) override;
  virtual BrokerStub stub() override;
  virtual Clock relay(const Data& entry, Port sender) override;

private:
  std::unique_ptr<BrokerGrpcStub> _stub;
};

}

#endif
