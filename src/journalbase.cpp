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
#include "journalbase.h"
#include <cassert>

namespace Cashmere
{

Random JournalBase::_random{};

JournalBase::JournalBase()
  : JournalBase(_random.next())
{
}

JournalBase::JournalBase(Id id)
  : Broker()
  , _id(id)
  , _bookId(_random.next())
{
}

JournalBase::~JournalBase() {}

Id JournalBase::id() const
{
  return _id;
}

Id JournalBase::bookId() const
{
  return _bookId;
}

bool JournalBase::append(Amount value)
{
  return append({id(), value, {}});
}

bool JournalBase::append(const Data& entry)
{
  return insert({clock().tick(entry.id), entry}).valid();
}

Clock JournalBase::insert(const Entry& data, Port port)
{
  if (save(data)) {
    return Broker::insert(data, port);
  }
  return Clock();
}

bool JournalBase::replace(Amount value, const Clock& clock)
{
  return append({id(), value, clock});
}

bool JournalBase::erase(Clock time)
{
  return append({id(), 0, time});
}

bool JournalBase::contains(const Clock& time) const
{
  return entry(time).valid();
}

EntryList JournalBase::query(const Clock& from, Port) const
{
  EntryList list;
  for (const auto& [clock, entry] : entries()) {
    if (clock.concurrent(from) || from.smallerThan(clock)) {
      list.push_back({clock, entry});
    }
  }
  return list;
}

}
