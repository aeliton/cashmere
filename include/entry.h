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
#include <signal/signal.h>

namespace Cashmere
{

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

using ClockEntryMap = std::map<Clock, Entry>;
using ReplaceEntryMap = std::map<Clock, ClockEntry>;
using ClockEntryList = std::list<ClockEntry>;

using ClockChangeSignal = Signal<bool(ClockEntry)>;
using ClockChangeSlot = ClockChangeSignal::Slot;

class EntryHandler
{
public:
  explicit EntryHandler(Id id);
  virtual ~EntryHandler() = 0;
  virtual bool insert(const ClockEntry& data) = 0;
  virtual bool insert(const ClockEntryList& entries);

  Id id() const;
  Clock clock() const;

  ClockChangeSignal& clockChanged();

protected:
  void setClock(const Clock& clock);
  void clockTick(Id id);

private:
  const Id _id;
  Clock _clock;
  ClockChangeSignal _clockChanged;
};

}

#endif
