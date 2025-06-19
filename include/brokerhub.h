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
#ifndef CASHMERE_BROKER_HUB_H
#define CASHMERE_BROKER_HUB_H

#include "brokerI.h"

#include <vector>

namespace Cashmere
{

class BrokerHub;
using BrokerHubPtr = std::shared_ptr<BrokerHub>;

class BrokerHub : public BrokerI
{
public:
  BrokerHub();

  Id id() const override;
  Clock insert(const EntryList& entries, Port sender = 0) override
  {
    return {};
  }
  Clock insert(const Entry& data, Port sender = 0) override;
  EntryList entries(const Clock& from = {}) const override
  {
    return {};
  }
  EntryList entries(const Clock& from, Port ignore) const override
  {
    return {};
  }
  IdConnectionInfoMap provides(Port to = 0) const override
  {
    return {};
  }
  IdClockMap versions() const override
  {
    return {};
  }
  Clock clock() const override
  {
    return {};
  }
  Port connect(BrokerIPtr other) override;
  Port disconnect(Port port) override
  {
    return {};
  }
  BrokerIPtr ptr() override
  {
    return {};
  }
  void setClock(const Clock& clock) override
  {
    return;
  }
  void connect(BrokerIPtr source, Port local, Port remote) override
  {
    return;
  }
  Port getLocalPortFor(BrokerIPtr broker) override
  {
    return {};
  }

  std::set<Port> connectedPorts() const override;

private:
  std::vector<BrokerIWeakPtr> _connections;
};

}
#endif
