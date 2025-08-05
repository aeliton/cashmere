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
#ifndef CASHMERE_ENTRY_H
#define CASHMERE_ENTRY_H

#include "clock.h"

#include <list>

namespace Cashmere
{

struct Data;
struct Entry;

using ClockDataMap = std::map<Clock, Data>;
using ClockEntryMap = std::map<Clock, Entry>;
using EntryList = std::list<Entry>;

struct CASHMERE_EXPORT Data
{
  Id id;
  Amount value;
  Clock alters;
  bool operator==(const Data& other) const;
  bool valid() const;
  static bool Read(std::istream& in, Data& data);
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const Data& data);
};

struct CASHMERE_EXPORT Entry
{
  Clock clock;
  Data entry;
  bool operator==(const Entry& other) const;
  static bool Read(std::istream& in, Entry& entry);
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const Entry& data);
};

}

#endif
