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

JournalBase::~JournalBase() = default;

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

Journal::~Journal() {}

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

bool Journal::append(Amount value)
{
  return append({_id, value, {}});
}

bool Journal::append(Entry value)
{
  _clock[value.journalId]++;
  insert(_clock, value);
  _clockChanged(_clock, value);
  return true;
}

bool Journal::insert(Clock clock, Entry value)
{
  if (_entries.find(clock) != _entries.end()) {
    return false;
  }
  _entries[clock] = value;
  _clock = _clock.merge(clock);
  return true;
}

bool Journal::replace(Amount value, const Clock& clock)
{
  return append({_id, value, clock});
}

bool Journal::erase(Clock time)
{
  return append({_id, 0, time});
}

Entry Journal::query(Clock time) const
{
  if (_entries.find(time) == _entries.end()) {
    return {0, 0, {{0UL, 0}}};
  }

  return _entries.at(time);
}

const JournalEntries& Journal::entries() const
{
  return _entries;
}

ClockChangeSignal& Journal::clockChanged()
{
  return _clockChanged;
}
}
