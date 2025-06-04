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
Ledger::Ledger(JournalBasePtr journal)
  : Ledger(journal->entries())
{
  _journal = journal;
}

Ledger::Ledger(const ClockEntryList& entries)
  : _journal(nullptr)
  , _balance(0)
{
  for (auto& clockEntry : entries) {
    auto [act, what] = action(clockEntry);
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

Amount Ledger::balance(const ClockEntryList& entries)
{
  Ledger ledger(entries);
  return ledger.balance();
}

bool Ledger::replaces(const ClockEntry& existing, const ClockEntry& incoming)
  const
{
  if (existing.clock.smallerThan(incoming.clock)) {
    return true;
  }
  if (incoming.clock.smallerThan(existing.clock)) {
    return false;
  }
  return existing.entry.journalId < incoming.entry.journalId;
}

std::tuple<Ledger::Action, Clock> Ledger::action(const ClockEntry& incoming
) const
{
  const auto& [clock, entry] = incoming;
  const bool isInsert = entry.alters.empty();
  if (isInsert && _rows.find(clock) != _rows.end()) {
    return {Action::Ignore, {}};
  }
  const auto key = isInsert ? clock : entry.alters;
  if (_rows.find(key) == _rows.end()) {
    return {Action::Insert, key};
  }
  if (replaces(_rows.at(key), {clock, entry})) {
    return {Action::Replace, key};
  }
  return {Action::Ignore, {}};
}
}
