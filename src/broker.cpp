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

Broker::Broker(Id id)
  : EntryHandler(id)
{
}

IdClockMap Broker::versions() const
{
  return _versions;
}

bool Broker::attach(JournalBasePtr journal)
{
  if (!journal) {
    return false;
  }

  const Clock& from = _versions[journal->id()];

  const auto entriesJournal = journal->entries(from);
  ClockEntryList entriesProvider = {};
  if (const auto provider = pickAttached()) {
    entriesProvider = provider->entries(from);
  }

  for (const auto& [id, context] : _attached) {
    auto attached = context.journal.lock();
    if (attached && attached->insert(entriesJournal)) {
      _versions[attached->id()] = attached->clock();
    }
  }

  journal->insert(entriesProvider);

  _versions[journal->id()] = journal->clock();

  _attached[journal->id()] = AttachContext{
    journal, journal->clockChanged().connect(this, &Broker::insert)
  };

  setClock(clock().merge(journal->clock()));

  return true;
}

bool Broker::detach(Id journalId)
{
  if (_attached.find(journalId) == _attached.end()) {
    return false;
  }
  const auto& [ref, conn] = _attached[journalId];
  if (auto journal = ref.lock()) {
    journal->clockChanged().disconnect(conn);
  }
  _attached.erase(journalId);
  return true;
}

bool Broker::insert(const ClockEntry& data)
{
  _versions[data.entry.journalId] = data.clock;
  for (const auto& [id, context] : _attached) {
    if (id == data.entry.journalId) {
      continue;
    }
    auto journal = context.journal.lock();
    if (journal && journal->insert(data)) {
      _versions[journal->id()] = _versions[journal->id()].merge(data.clock);
    }
  }
  setClock(clock().merge(data.clock));
  return false;
}

IdSet Broker::attachedIds() const
{
  const auto it = std::views::keys(_attached);
  return {it.begin(), it.end()};
}

JournalBasePtr Broker::pickAttached() const
{
  for (const auto& [id, context] : _attached) {
    if (auto other = context.journal.lock()) {
      return other;
    }
  }
  return nullptr;
}

}
