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

#include "brokerbase.h"
#include <memory>
#include <vector>

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
using BrokerWeakPtr = std::weak_ptr<Broker>;

class Broker : public std::enable_shared_from_this<Broker>, public BrokerBase
{
public:
  Broker();

  virtual ~Broker();

  virtual Id id() const override;
  virtual Clock clock() const override;
  virtual IdClockMap versions() const override;
  virtual IdConnectionInfoMap provides(Port sender = 0) const override;
  virtual Clock insert(const Entry& data, Port sender = 0) override;
  virtual EntryList
  entries(const Clock& from = {}, Port sender = 0) const override;

  virtual Port connect(BrokerBasePtr other) override;
  virtual Port disconnect(Port port) override;
  virtual bool refresh(const Connection& conn, Port port) override;
  virtual std::set<Port> connectedPorts() const override;

  virtual BrokerBasePtr ptr() override;

private:
  void refreshConnections(Port ignore = 0);
  void setClock(const Clock& clock) override;
  Connection connect(Connection conn) override;

  std::vector<Connection> _connections;
};

}

#endif
