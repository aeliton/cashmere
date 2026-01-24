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
#include "cashmere/journalbase.h"
#include <cassert>

namespace Cashmere
{

JournalBase::JournalBase(const std::string& url)
  : Broker(url)
  , _bookId(0)
{
}

JournalBase::~JournalBase() {}

Id JournalBase::bookId() const
{
  return _bookId;
}

Clock JournalBase::insert(const Entry& data, Source source)
{
  if (!data.clock.isNext(clock(), data.entry.id)) {
    return Clock{{0, 0}};
  }
  if (save(data)) {
    return Broker::insert(data, source);
  }
  return Clock{{0, 0}};
}

EntryList JournalBase::query(const Clock& from, Source) const
{
  EntryList list;
  for (const auto& [clock, entry] : entries()) {
    if (clock.concurrent(from) || from.smallerThan(clock)) {
      list.push_back({clock, entry});
    }
  }
  return list;
}

SourcesMap JournalBase::sources(Source sender) const
{
  auto out = Broker::sources(sender);
  out[0] = {{id(), {0, clock()}}};
  return out;
}

Clock JournalBase::relay(const Data& data, Source sender)
{
  if (data.id == 0) {
    return Broker::relay({id(), data.value, data.alters}, sender);
  }
  return Broker::relay(data, sender);
}
}
