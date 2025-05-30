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
#ifndef CASHEMERE_JOURNAL_H
#define CASHEMERE_JOURNAL_H

#include <map>
#include <memory>
#include <signal/signal.h>

#include "cashmere.h"
#include "clock.h"
#include "random.h"

namespace Cashmere
{

using ClockChangeSignal = Signal<void(Clock)>;
using ClockChangeSlot = ClockChangeSignal::Slot;

class Journal
{
public:
  struct Entry
  {
    Id journalId;
    Amount value;
    Clock alters;
    friend bool operator==(const Entry& l, const Entry& r)
    {
      return std::tie(l.value, l.alters) == std::tie(r.value, r.alters);
    }
    bool valid() const
    {
      return alters.size() > 0 && alters.begin()->first != 0UL;
    }
  };

  using JournalEntries = std::map<Clock, Entry>;

  Journal();
  explicit Journal(Id id);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(Id journalId, Amount value);
  bool append(Entry value);
  bool insert(Clock clock, Entry value);

  bool replace(Amount value, const Clock& clock);
  bool replace(Id journalId, Amount value, const Clock& clock);

  bool erase(Clock time);
  bool erase(Id journalId, Clock time);

  Entry query(Clock time) const;

  const JournalEntries& entries() const;

  bool connect(ClockChangeSlot func);

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  JournalEntries _entries;
  Clock _clock;
  ClockChangeSignal _clockChanged;
};

using JournalPtr = std::shared_ptr<Journal>;

}

#endif
