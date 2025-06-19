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
#ifndef CASHMERE_BROKER_H
#define CASHMERE_BROKER_H

#include "brokerI.h"
#include <memory>
#include <vector>

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
using BrokerWeakPtr = std::weak_ptr<Broker>;

struct Context;
using ContextPtr = std::shared_ptr<Context>;

class Broker : public std::enable_shared_from_this<Broker>, public BrokerI
{
public:
  Broker();

  virtual ~Broker();

  virtual Id id() const override;
  Clock insert(const EntryList& entries, Port sender = 0) override;
  virtual Clock insert(const Entry& data, Port sender = 0) override;

  virtual EntryList entries(const Clock& from = {}) const override;
  EntryList entries(const Clock& from, Port ignore) const override;

  virtual IdConnectionInfoMap provides(Port to = 0) const override;
  IdClockMap versions() const override;

  Clock clock() const override;

  Port connect(BrokerIPtr other) override;
  Port disconnect(Port port) override;

  BrokerIPtr ptr() override;
  std::set<Port> connectedPorts() const override
  {
    return {};
  }

private:
  void setClock(const Clock& clock) override;
  void connect(BrokerIPtr source, Port local, Port remote) override;
  Port getLocalPortFor(BrokerIPtr broker) override;

  std::vector<ContextPtr> _contexts;
};

}

#endif
