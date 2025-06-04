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

Journal::Journal(Id id, const ClockEntryMap& entries)
  : _bookId(_random.next())
  , _id(id)
  , _clock({})
  , _entries(entries)
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

bool Journal::append(const Entry& entry)
{
  _clock[entry.journalId]++;
  insert(_clock, entry);
  _clockChanged(_clock, entry);
  return true;
}

bool Journal::insert(const Clock& clock, const Entry& entry)
{
  if (_entries.find(clock) != _entries.end()) {
    return false;
  }
  _entries[clock] = entry;
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

ClockChangeSignal& Journal::clockChanged()
{
  return _clockChanged;
}

}
