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
struct ClockEntry
{
  Clock clock;
  Journal::Entry entry;
};

Ledger::Ledger(JournalPtr journal)
  : _journal(journal)
{
}

Amount Ledger::balance() const
{
  Amount result = 0;
  std::map<Clock, ClockEntry> rows;

  auto existingKeyNeedsReplace = [&rows](const Clock& k, const Clock& c,
                                     const Journal::Entry& e) -> bool {
    if (rows.find(k) == rows.end()) {
      return false;
    }
    if (rows.at(k).clock.smallerThan(c)) {
      return true;
    }
    if (c.smallerThan(rows.at(k).clock)) {
      return false;
    }
    return rows[k].entry.journalId < e.journalId;
  };

  for (auto& [clock, entry] : _journal->entries()) {
    const bool isInsert = entry.alters.empty();
    if (isInsert && rows.find(clock) != rows.end()) {
      continue;
    }
    const auto key = isInsert ? clock : entry.alters;
    const bool keyExists = rows.find(key) != rows.end();
    if (keyExists) {
      if (!existingKeyNeedsReplace(key, clock, entry)) {
        continue;
      }
      result -= rows.at(key).entry.value;
    }
    rows[key] = {clock, entry};
    result += rows.at(key).entry.value;
  }
  return result;
}
}
