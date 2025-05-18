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

Ledger::Ledger(JournalPtr journal)
  : _journal(journal)
{
}

Amount Ledger::balance() const
{
  std::map<Journal::Clock, std::pair<Journal::Clock, Journal::Entry>> applied;
  Amount result = 0;
  for (auto& [clock, entry] : _journal->journals()) {
    if (entry.value != 0 && entry.alters.empty()) {
      if (applied.find(clock) == applied.end()) {
        applied[clock] = {clock, entry};
        result += applied.at(clock).second.value;
      }
    } else {
      auto q = applied.find(entry.alters);
      if (q == applied.end()) {
        applied[entry.alters] = {clock, entry};
      } else {
        auto [oldClock, oldEntry] = q->second;
        if (oldClock < clock) {
          result -= oldEntry.value;
          applied[entry.alters] = {clock, entry};
        }
      }
      result += applied.at(entry.alters).second.value;
    }
  }
  return result;
}
}
