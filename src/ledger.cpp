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
#include "ledger.h"

namespace Cashmere
{
Ledger::Ledger(BrokerPtr journal)
  : Ledger(journal->query())
{
  _journal = journal;
}

Ledger::Ledger(const EntryList& entries)
  : _journal(nullptr)
  , _balance(0)
{
  for (auto& clockEntry : entries) {
    auto [act, what] = Evaluate(_rows, clockEntry);
    switch (act) {
      case Action::Replace:
        _balance -= _rows.at(what).entry.value;
        [[fallthrough]];
      case Action::Insert:
        _rows[what] = clockEntry;
        _balance += _rows.at(what).entry.value;
        break;
      case Action::Ignore:
        break;
    }
  }
}

Amount Ledger::balance() const
{
  return _balance;
}

Amount Ledger::Balance(const EntryList& entries)
{
  Ledger ledger(entries);
  return ledger.balance();
}

Ledger::ActionClock
Ledger::Replaces(const Entry& existing, const Entry& incoming)
{
  if (incoming.entry.alters.empty()) {
    return {Action::Ignore, {}};
  }
  if (existing.clock.smallerThan(incoming.clock)) {
    return {Action::Replace, incoming.entry.alters};
  }
  if (incoming.clock.smallerThan(existing.clock)) {
    return {Action::Ignore, {}};
  }
  if (existing.entry.id < incoming.entry.id) {
    return {Action::Replace, incoming.entry.alters};
  }
  return {Action::Ignore, {}};
}

Ledger::ActionClock
Ledger::Evaluate(const ClockEntryMap& rows, const Entry& incoming)
{
  if (incoming.entry.alters.empty()) {
    if (rows.find(incoming.clock) == rows.end()) {
      return {Action::Insert, {incoming.clock}};
    }
    return {Action::Ignore, {}};
  }

  if (rows.find(incoming.entry.alters) == rows.end()) {
    return {Action::Insert, incoming.entry.alters};
  }

  return Replaces(rows.at(incoming.entry.alters), incoming);
}

}
