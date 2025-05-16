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
  std::map<Journal::Entry::Id, Journal::Entry> applied;
  Amount result = 0;
  for (auto& [journalId, entries] : _journal->journals()) {
    for (auto& [time, entry] : entries) {
      if (entry.operation == Journal::Operation::Insert) {
        auto entryId = Journal::Entry::Id{journalId, time};
        applied[entryId] = entry;
        result += applied.at(entryId).value;
      } else {
        auto entryId = entry.alters;
        result -= applied.at(entryId).value;
        applied[entryId] = entry;
        result += applied.at(entryId).value;
      }
    }
  }
  return result;
}
}
