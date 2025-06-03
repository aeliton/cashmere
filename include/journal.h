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

#include <memory>
#include <signal/signal.h>

#include "entry.h"
#include "random.h"

namespace Cashmere
{

using ClockChangeSignal = Signal<void(Clock, Entry)>;
using ClockChangeSlot = ClockChangeSignal::Slot;
using JournalEntries = std::map<Clock, Entry>;

class JournalBase
{
public:
  virtual ~JournalBase() = 0;
  virtual const JournalEntries& entries() const = 0;
};

class Journal : public JournalBase
{
public:
  Journal();
  ~Journal();
  explicit Journal(Id id);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(Entry value);
  bool insert(Clock clock, Entry value);

  bool replace(Amount value, const Clock& clock);

  bool erase(Clock time);

  Entry query(Clock time) const;

  const JournalEntries& entries() const override;

  ClockChangeSignal& clockChanged();

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  JournalEntries _entries;
  Clock _clock;
  ClockChangeSignal _clockChanged;
};

using JournalBasePtr = std::shared_ptr<JournalBase>;
using JournalPtr = std::shared_ptr<Journal>;

}

#endif
