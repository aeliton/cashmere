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

#include "entry.h"
#include "random.h"

namespace Cashmere
{

struct JournalBase : public EntryHandler
{
  virtual ~JournalBase() = 0;
  virtual const Id id() const = 0;
  virtual Clock clock() const = 0;
  virtual ClockEntryList entries(const Clock& from = {}) const = 0;
};

class Journal : public JournalBase
{
public:
  Journal();
  ~Journal();
  explicit Journal(Id id, const ClockEntryMap& entries = {});

  const Id id() const override;
  Clock clock() const override;
  bool insert(const Clock& clock, const Entry& entry) override;
  ClockEntryList entries(const Clock& from = {}) const override;
  ClockChangeSignal& clockChanged() override;

  const Id bookId() const;
  bool append(Amount value);
  bool append(const Entry& entry);
  bool replace(Amount value, const Clock& clock);
  bool erase(Clock time);
  bool contains(const Clock& clock) const;
  Entry entry(Clock time) const;

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  ClockEntryMap _entries;
  Clock _clock;
  ClockChangeSignal _clockChanged;
};

using JournalBasePtr = std::shared_ptr<JournalBase>;
using JournalPtr = std::shared_ptr<Journal>;

}

#endif
