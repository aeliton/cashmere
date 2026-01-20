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

#include "cashmere/broker.h"
#include <thread>

namespace Cashmere
{

class BrokerGrpc;
using BrokerGrpcPtr = std::shared_ptr<BrokerGrpc>;
using BrokerGrpcWeakPtr = std::weak_ptr<BrokerGrpc>;

class CASHMERE_EXPORT BrokerGrpc : public Broker
{
public:
  BrokerGrpc(const std::string& hostname, uint16_t port);

  ~BrokerGrpc();
  static BrokerBasePtr create(const std::string& input);

  std::thread start();
  void stop();

  Connection stub() override;

  std::string schema() const override;

private:
  std::string _hostname;
  uint16_t _port;

  class Impl;
  std::unique_ptr<Impl> _impl;
};

}

#endif
