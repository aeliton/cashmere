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
#include "brokerhub.h"

namespace Cashmere
{

BrokerHub::BrokerHub()
{
  _connections.push_back({nullptr, 0});
}

Id BrokerHub::id() const
{
  return 0;
}

Port BrokerHub::connect(BrokerIPtr broker)
{
  const Port port = _connections.size();
  _connections.push_back({broker, broker->getLocalPortFor({ptr(), port})});
  return port;
}

Clock BrokerHub::insert(const Entry& data, Port sender)
{
  for (size_t i = 1; i < _connections.size(); i++) {
    if (i == sender) {
      continue;
    }
    _connections.at(i).insert(data);
  }
  return _connections.at(0).merge(data.clock);
}

std::set<Port> BrokerHub::connectedPorts() const
{
  std::set<Port> connected;
  for (size_t i = 1; i < _connections.size(); i++) {
    connected.insert(i);
  }
  return connected;
}

Port BrokerHub::getLocalPortFor(Connection conn)
{
  Port port = _connections.size();
  _connections.push_back(conn);
  return port;
}

BrokerIPtr BrokerHub::ptr()
{
  return shared_from_this();
}

Clock BrokerHub::clock() const
{
  return _connections.front().version();
}
}
