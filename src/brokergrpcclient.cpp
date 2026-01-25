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
#include "cashmere/brokergrpcclient.h"
#include "brokergrpcstub.h"

namespace Cashmere
{
BrokerGrpcClient::BrokerGrpcClient(const std::string& url)
  : BrokerBase()
  , _stub(std::make_unique<BrokerGrpcStub>(url))
{
}

BrokerGrpcClient::~BrokerGrpcClient() = default;

Clock BrokerGrpcClient::clock() const
{
  return _stub->clock();
}

IdClockMap BrokerGrpcClient::versions() const
{
  return _stub->versions();
}

SourcesMap BrokerGrpcClient::sources(Source sender) const
{
  return _stub->sources(sender);
}

Clock BrokerGrpcClient::insert(const Entry& data, Source sender)
{
  return _stub->insert(data, sender);
}

EntryList BrokerGrpcClient::query(const Clock& from, Source sender) const
{
  return _stub->query(from, sender);
}

Connection BrokerGrpcClient::connect(Connection conn)
{
  return _stub->connect(conn);
}

bool BrokerGrpcClient::refresh(const Connection& conn, Source sender)
{
  return _stub->refresh(conn, sender);
}

Clock BrokerGrpcClient::relay(const Data& entry, Source sender)
{
  return _stub->relay(entry, sender);
}

Connection BrokerGrpcClient::stub()
{
  return _stub->stub();
}
}
