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

#include "cashmere/cashmere.h"
#include "cashmere/entry.h"
#include "cashmere/brokerbase.h"

namespace Cashmere
{

using ClockEntryMap = std::map<Clock, Entry>;

class CASHMERE_EXPORT Ledger
{
public:
  enum class Action
  {
    Ignore,
    Insert,
    Replace
  };
  using ActionClock = std::tuple<Ledger::Action, Clock>;
  Ledger() = delete;
  explicit Ledger(BrokerBasePtr journal);

  Amount balance() const;

  static Amount Balance(const EntryList& entries);
  static ActionClock Evaluate(const ClockEntryMap& rows, const Entry& incoming);
  static ActionClock Replaces(const Entry& existing, const Entry& incoming);

private:
  explicit Ledger(const EntryList& entries);
  BrokerBasePtr _journal;
  Amount _balance;
  ClockEntryMap _rows;
};

}

#endif
