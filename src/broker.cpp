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
#include "broker.h"

namespace Cashmere
{

Broker::Broker() {}

std::map<Id, Clock> Broker::versions() const
{
  return _versions;
}

bool Broker::attach(JournalPtr journal)
{
  Id journalId = journal->id();

  _attached[journalId] = journal;
  if (auto clock = journal->clock(); clock.size() > 0) {
    _versions[journal->id()] = clock;
  }

  journal->connect([this, journalId](Clock clock) {
    if (auto journal = this->_attached[journalId].lock()) {
      this->_versions[journalId] = clock;
    }
    if (auto sender = this->_attached[journalId].lock()) {
      auto entry = sender->query(clock);
      for (auto& [id, weakJournalPtr] : this->_attached) {
        if (id == sender->id()) {
          continue;
        }
        if (auto other = weakJournalPtr.lock()) {
          other->insert(clock, entry);
        }
      }
    }
  });
  return true;
}
}
