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

#include "entry.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
using BrokerWeakPtr = std::weak_ptr<Broker>;
struct Context;
using ContextPtr = std::shared_ptr<Context>;

struct ConnectionInfo
{
  int64_t distance;
  Clock version;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

class Broker : public std::enable_shared_from_this<Broker>
{
public:
  enum class Type
  {
    Transport,
    Store
  };

  Broker();

  virtual ~Broker();

  virtual Clock insert(const Entry& data, Port sender = 0);
  virtual Clock insert(const EntryList& entries, Port sender = 0);
  virtual EntryList entries(const Clock& from = {}) const;
  virtual IdConnectionInfoMap provides(Port to = 0) const;
  virtual IdClockMap versions() const;
  virtual Type type() const;

  Clock clock() const;

  bool attach(BrokerPtr other);
  bool detach(Port port);

  BrokerPtr ptr();

private:
  void setClock(const Clock& clock);
  void attach(BrokerPtr source, Port local, Port remote);
  Port getLocalPortFor(BrokerPtr broker);
  EntryList entries(const Clock& from, Port ignore) const;

  static IdConnectionInfoMap UpdateProvides(IdConnectionInfoMap provides);

  std::vector<ContextPtr> _contexts;
};

}

#endif
