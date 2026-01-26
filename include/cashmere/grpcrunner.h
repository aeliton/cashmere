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

#include "cashmere/cashmere.h"

#include <memory>
#include <thread>

namespace Cashmere
{

class GrpcRunner;
using GrpcRunnerPtr = std::shared_ptr<GrpcRunner>;
using GrpcRunnerWeakPtr = std::weak_ptr<GrpcRunner>;

class CASHMERE_EXPORT GrpcRunner
{
public:
  GrpcRunner(const std::string& hostport, BrokerBasePtr broker);

  ~GrpcRunner();

  std::thread start();
  void stop();

private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

}

#endif
