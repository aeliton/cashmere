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

Broker::Broker()
  : EntryHandler()
{
}

IdClockMap Broker::versions() const
{
  IdClockMap out;
  for (auto& [id, index] : _idToContext) {
    out[id] = _attached[index].version;
  }
  return out;
}

bool Broker::attach(EntryHandlerPtr journal)
{
  if (!journal) {
    return false;
  }

  Id sample = *journal->provides().begin();
  const AttachContext context = _idToContext.find(sample) != _idToContext.cend()
                                  ? _attached[_idToContext.at(sample)]
                                  : AttachContext{journal, {}, 0};

  const auto entriesJournal = journal->entries(context.version);
  ClockEntryList entriesProvider = {};
  if (const auto provider = pickAttached()) {
    entriesProvider = provider->entries(context.version);
  }

  for (auto& context : _attached) {
    auto journal = context.journal.lock();
    if (journal && journal->insert(entriesJournal)) {
      context.version = journal->clock();
    }
  }

  journal->insert(entriesProvider);

  if (_idToContext.find(sample) == _idToContext.cend()) {
    for (auto id : journal->provides()) {
      _idToContext[id] = _attached.size();
    }
    _attached.push_back(context);
    _attached.back().conn =
      journal->clockChanged().connect(this, &Broker::insert);
    _attached.back().version = journal->clock();
  } else {
    _attached[_idToContext.at(sample)].version = journal->clock();
  }

  setClock(clock().merge(journal->clock()));

  return true;
}

bool Broker::detach(Id journalId)
{
  if (_idToContext.find(journalId) == _idToContext.end()) {
    return false;
  }
  auto& context = _attached[_idToContext.at(journalId)];
  if (auto journal = context.journal.lock()) {
    journal->clockChanged().disconnect(context.conn);
    context.journal.reset();
    return true;
  }
  return false;
}

bool Broker::insert(const ClockEntry& data)
{
  if (_idToContext.find(data.entry.journalId) == _idToContext.cend()) {
    _idToContext[data.entry.journalId] = _attached.size();
    _attached.push_back(
      AttachContext{std::weak_ptr<EntryHandler>(), data.clock, 0}
    );
  }
  _attached[_idToContext.at(data.entry.journalId)].version = data.clock;

  for (auto& context : _attached) {
    auto journal = context.journal.lock();
    if (!journal) {
      continue;
    }
    const auto provided = journal->provides();
    if (provided.find(data.entry.journalId) != provided.cend()) {
      continue;
    }
    if (journal->insert(data)) {
      context.version = context.version.merge(data.clock);
    }
  }
  setClock(clock().merge(data.clock));
  return false;
}

IdSet Broker::provides() const
{
  IdSet out;
  for (auto& attached : _attached) {
    if (auto journal = attached.journal.lock()) {
      auto provided = journal->provides();
      out.insert(provided.begin(), provided.end());
    }
  }
  return out;
}

EntryHandlerPtr Broker::pickAttached() const
{
  for (const auto& context : _attached) {
    if (auto other = context.journal.lock()) {
      return other;
    }
  }
  return nullptr;
}

}
