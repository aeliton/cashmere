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
#ifndef CASHMERE_LEDGER_H
#define CASHMERE_LEDGER_H

#include "journal.h"

namespace Cashmere
{

class Ledger
{
public:
  enum class Action
  {
    Ignore,
    Insert,
    Replace
  };
  Ledger() = delete;
  explicit Ledger(JournalBasePtr journal);

  Amount balance() const;
  static Amount balance(const JournalEntries& entries);

  std::tuple<Action, Clock> action(const ClockEntry& incoming) const;
  bool replaces(const ClockEntry& existing, const ClockEntry& incoming) const;

private:
  explicit Ledger(const JournalEntries& entries);
  JournalBasePtr _journal;
  Amount _balance;
  ClockEntryMap _rows;
};

}

#endif
