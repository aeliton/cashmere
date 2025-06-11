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
#ifndef CASHMERE_ENTRY_H
#define CASHMERE_ENTRY_H

#include "clock.h"

#include <list>
#include <memory>
#include <signal/signal.h>

namespace Cashmere
{

struct Entry;
struct ClockEntry;
class EntryHandler;

using ClockEntryMap = std::map<Clock, Entry>;
using ReplaceEntryMap = std::map<Clock, ClockEntry>;
using ClockEntryList = std::list<ClockEntry>;
using Port = int64_t;

using ClockChangeSignal = Signal<bool(ClockEntry)>;
using ClockChangeSlot = ClockChangeSignal::Slot;
using EntryHandlerPtr = std::shared_ptr<EntryHandler>;

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
  Context(std::shared_ptr<EntryHandler> j, const Clock& v, Connection c);
  std::weak_ptr<EntryHandler> journal;
  Clock version;
  Port conn;
  IdDistanceMap provides;
};

struct Entry
{
  Id journalId;
  Amount value;
  Clock alters;
  friend bool operator==(const Entry& l, const Entry& r)
  {
    return std::tie(l.journalId, l.value, l.alters) ==
           std::tie(r.journalId, r.value, r.alters);
  }
  bool valid() const
  {
    return alters.size() > 0 && alters.begin()->first != 0UL;
  }
};

struct ClockEntry
{
  Clock clock;
  Entry entry;
  const bool operator==(const ClockEntry& other) const;
};

class EntryHandler : public std::enable_shared_from_this<EntryHandler>
{
public:
  explicit EntryHandler(Id id = 0);
  virtual ~EntryHandler() = 0;
  virtual bool insert(const ClockEntry& data, Port sender = 0);
  virtual bool insert(const ClockEntryList& entries, Port sender = 0);
  virtual ClockEntryList entries(const Clock& from = {}) const;
  virtual IdDistanceMap provides() const;

  Id id() const;
  Clock clock() const;

  bool attach(EntryHandlerPtr other);
  bool detach(Port port);
  virtual IdClockMap versions() const;

  EntryHandlerPtr ptr();

protected:
  void setClock(const Clock& clock);
  void clockTick(Id id);
  Port attach(EntryHandlerPtr source, Port port);

private:
  const Id _id;
  std::vector<ContextPtr> _contexts;
  std::unordered_map<Id, ContextPtr> _contextMap;
};

}

#endif
