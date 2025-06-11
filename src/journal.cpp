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
#include "journal.h"
#include <cassert>

namespace Cashmere
{

Random Journal::_random{};

Journal::Journal()
  : Journal(_random.next())
{
}

Journal::Journal(Id id, const ClockEntryMap& entries)
  : EntryHandler(id)
  , _bookId(_random.next())
  , _entries(entries)
{
}

Journal::~Journal() {}

const Id Journal::bookId() const
{
  return _bookId;
}

bool Journal::append(Amount value)
{
  return append({id(), value, {}});
}

bool Journal::append(const Entry& entry)
{
  clockTick(entry.journalId);
  return insert({clock(), entry});
}

bool Journal::insert(const ClockEntry& data, Port port)
{
  if (_entries.find(data.clock) != _entries.end()) {
    return false;
  }
  _entries[data.clock] = data.entry;
  setClock(clock().merge(data.clock));
  return EntryHandler::insert(data, port);
}

bool Journal::replace(Amount value, const Clock& clock)
{
  return append({id(), value, clock});
}

bool Journal::erase(Clock time)
{
  return append({id(), 0, time});
}
bool Journal::contains(const Clock& time) const
{
  return _entries.find(time) != _entries.cend();
}

Entry Journal::entry(Clock time) const
{
  if (_entries.find(time) == _entries.end()) {
    return {0, 0, {{0UL, 0}}};
  }

  return _entries.at(time);
}

ClockEntryList Journal::entries(const Clock& from) const
{
  ClockEntryList list;
  for (const auto& [clock, entry] : _entries) {
    if (clock.concurrent(from) || from.smallerThan(clock)) {
      list.push_back({clock, entry});
    }
  }
  return list;
}

IdDistanceMap Journal::provides() const
{
  return {{id(), {.distance = 0}}};
}
}
