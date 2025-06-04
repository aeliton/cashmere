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

Ledger::Ledger(const JournalEntries& entries)
  : _journal(nullptr)
  , _balance(0)
{
  for (auto& [clock, entry] : entries) {
    auto [act, what] = action(clock, entry);
    switch (act) {
      case Action::Replace:
        _balance -= _rows.at(what).entry.value;
        [[fallthrough]];
      case Action::Insert:
        _rows[what] = {clock, entry};
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

Amount Ledger::balance(const JournalEntries& entries)
{
  Ledger ledger(entries);
  return ledger.balance();
}

bool Ledger::existingKeyNeedsReplace(
  const Clock& k, const Clock& c, const Entry& e
) const
{
  if (_rows.find(k) == _rows.end()) {
    return false;
  }
  if (_rows.at(k).clock.smallerThan(c)) {
    return true;
  }
  if (c.smallerThan(_rows.at(k).clock)) {
    return false;
  }
  return _rows.at(k).entry.journalId < e.journalId;
}

std::tuple<Ledger::Action, Clock>
Ledger::action(const Clock& clock, const Entry& entry) const
{
  const bool isInsert = entry.alters.empty();
  if (isInsert && _rows.find(clock) != _rows.end()) {
    return {Action::Ignore, {}};
  }
  const auto key = isInsert ? clock : entry.alters;
  const bool keyExists = _rows.find(key) != _rows.end();
  if (keyExists) {
    if (!existingKeyNeedsReplace(key, clock, entry)) {
      return {Action::Ignore, {}};
    }
    return {Action::Replace, key};
  }
  return {Action::Insert, key};
}
}
