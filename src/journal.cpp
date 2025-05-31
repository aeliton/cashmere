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

Journal::Journal(Id id)
  : _bookId(_random.next())
  , _id(id)
  , _clock({})
{
}

const Id Journal::id() const
{
  return _id;
}

const Id Journal::bookId() const
{
  return _bookId;
}

Clock Journal::clock() const
{
  return _clock;
}

bool Journal::append(Id journalId, Amount value)
{
  return append({journalId, value, {}});
}

bool Journal::append(Amount value)
{
  return append({_id, value, {}});
}

bool Journal::append(Entry value)
{
  _clock[value.journalId]++;
  return insert(_clock, value);
}

bool Journal::insert(Clock clock, Entry value)
{
  if (_entries.find(clock) != _entries.end()) {
    return false;
  }
  _entries[clock] = value;
  _clock = _clock.merge(clock);
  _clockChanged(_clock);
  return true;
}

bool Journal::replace(Amount value, const Clock& clock)
{
  return append({_id, value, clock});
}

bool Journal::replace(Id journalId, Amount value, const Clock& clock)
{
  return append({journalId, value, clock});
}

bool Journal::erase(Clock time)
{
  return append({_id, 0, time});
}

bool Journal::erase(Id journalId, Clock time)
{
  return append({journalId, 0, time});
}

Journal::Entry Journal::query(Clock time) const
{
  if (_entries.find(time) == _entries.end()) {
    return {0, 0, {{0UL, 0}}};
  }

  return _entries.at(time);
}

const Journal::JournalEntries& Journal::entries() const
{
  return _entries;
}

bool Journal::connect(ClockChangeSlot func)
{
  _clockChanged.connect(func);
  return true;
}

}
