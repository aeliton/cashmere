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

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
struct Context;
using ContextPtr = std::shared_ptr<Context>;

struct JournalData
{
  int64_t distance;
  Clock version;
  friend bool operator==(const JournalData& l, const JournalData& r)
  {
    return std::tie(l.distance, l.version) == std::tie(r.distance, r.version);
  }
  friend bool operator<(const JournalData& l, const JournalData& r)
  {
    return std::tie(l.distance, l.version) < std::tie(r.distance, r.version);
  }
};

using IdDistanceMap = std::map<Id, JournalData>;

struct Context
{
  Context(std::shared_ptr<Broker> j, const Clock& v, Connection c);
  std::weak_ptr<Broker> journal;
  Clock version;
  Port conn;
  IdDistanceMap provides;
};

class Broker : public std::enable_shared_from_this<Broker>
{
public:
  explicit Broker(Id id = 0);
  virtual ~Broker();
  virtual bool insert(const ClockEntry& data, Port sender = 0);
  virtual bool insert(const ClockEntryList& entries, Port sender = 0);
  virtual ClockEntryList entries(const Clock& from = {}) const;
  virtual IdDistanceMap provides() const;

  Id id() const;
  Clock clock() const;

  bool attach(BrokerPtr other);
  bool detach(Port port);
  virtual IdClockMap versions() const;

  BrokerPtr ptr();

protected:
  void setClock(const Clock& clock);
  void clockTick(Id id);
  Port attach(BrokerPtr source, Port port);

private:
  const Id _id;
  std::vector<ContextPtr> _contexts;
  std::unordered_map<Id, ContextPtr> _contextMap;
};

}

#endif
