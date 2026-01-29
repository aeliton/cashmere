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
#include "cache.h"

namespace Cashmere
{

Journal::Journal(const std::string& url)
  : JournalBase(url)
{
}

bool Journal::save(const Entry& data)
{
  if (_entries.find(data.clock) != _entries.cend()) {
    return false;
  }
  _entries[data.clock] = data.entry;
  return true;
}

Data Journal::entry(Clock time) const
{
  if (_entries.find(time) == _entries.end()) {
    return {0, 0, {}};
  }
  return _entries.at(time);
}

EntryList Journal::entries() const
{
  EntryList list;
  for (const auto& [clock, entry] : _entries) {
    list.push_back({clock, entry});
  }
  return list;
}

std::string Journal::schema() const
{
  return "cache";
}

BrokerBase* Journal::create(const std::string& url)
{
  return new Journal(url);
}

extern "C" CASHMERE_EXPORT Cashmere::BrokerBase* create(const std::string& url)
{
  return new Cashmere::Journal(url);
}

}
