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

VersionMap Broker::versions() const
{
  return _versions;
}

bool Broker::attach(JournalBasePtr journal)
{
  Id journalId = journal->id();
  auto& lastVersion = _versions[journalId];

  bool sent = false;
  for (auto& [id, context] : _attached) {
    if (auto other = context.journal.lock()) {
      if (!sent) {
        for (auto& [clock, entry] : other->entries(lastVersion)) {
          journal->insert(clock, entry);
        }
        sent = true;
      }
      for (auto& [clock, entry] : journal->entries(lastVersion)) {
        other->insert(clock, entry);
      }
    }
  }

  _attached[journalId] = {
    journal, journal->clockChanged().connect(this, &Broker::onClockUpdate)
  };
  if (auto clock = journal->clock(); clock.size() > 0) {
    _versions[journal->id()] = clock;
  }

  return true;
}

bool Broker::detach(Id journalId)
{
  if (_attached.find(journalId) == _attached.end()) {
    return false;
  }
  auto& [ref, conn] = _attached[journalId];
  if (auto journal = ref.lock()) {
    journal->clockChanged().disconnect(conn);
  }
  _attached.erase(journalId);
  return true;
}

void Broker::onClockUpdate(Clock clock, Entry entry)
{
  _versions[entry.journalId] = clock;
  for (auto& [id, context] : _attached) {
    if (id == entry.journalId) {
      continue;
    }
    if (const auto journal = context.journal.lock()) {
      journal->insert(clock, entry);
      _versions[journal->id()] = journal->clock();
    }
  }
}

std::set<Id> Broker::attachedIds() const
{
  auto it = std::views::keys(_attached);
  return {it.begin(), it.end()};
}

}
