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

#include "cashmere/brokerbase.h"
#include "utils/random.h"
#include <memory>
#include <vector>

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
using BrokerWeakPtr = std::weak_ptr<Broker>;

class CASHMERE_EXPORT Broker : public std::enable_shared_from_this<Broker>,
                               public BrokerBase
{
public:
  Broker(Id id = 0UL);

  virtual ~Broker();

  virtual Clock clock() const override;
  virtual IdClockMap versions() const override;
  virtual SourcesMap sources(Source sender = 0) const override;
  virtual Clock insert(const Entry& data, Source sender = 0) override;
  virtual EntryList
  query(const Clock& from = {}, Source sender = 0) const override;

  Source disconnect(Source source);
  virtual bool refresh(const Connection& conn, Source source) override;
  virtual std::set<Source> connectedPorts() const;

  virtual BrokerBasePtr ptr();
  virtual Connection stub() override;

  Connection connect(Connection conn) override;

  Clock relay(const Data& entry, Source sender) override;

private:
  void refreshConnections(Source ignore = 0);
  void setClock(const Clock& clock);

  std::vector<Connection> _connections;
};

}

#endif
