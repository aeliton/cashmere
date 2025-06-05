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

  const auto entries = journal->entries(from);

  for (const auto& [id, context] : _attached) {
    if (auto attached = context.journal.lock()) {
      update(attached, entries);
    }
  }

  if (const auto provider = pickAttached()) {
    update(journal, provider->entries(from));
  }

  _attached[journal->id()] = AttachContext{
    journal, journal->clockChanged().connect(this, &Broker::onClockUpdate)
  };

  _versions[journal->id()] = journal->clock();

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

void Broker::onClockUpdate(Clock clock, Entry entry)
{
  _versions[entry.journalId] = clock;
  for (const auto& [id, context] : _attached) {
    if (id == entry.journalId) {
      continue;
    }
    if (const auto journal = context.journal.lock()) {
      journal->insert(clock, entry);
      _versions[journal->id()] = journal->clock();
    }
  }
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

void Broker::update(JournalBasePtr journal, const ClockEntryList& entries) const
{
  for (const auto& [clock, entry] : entries) {
    journal->insert(clock, entry);
  }
}
}
