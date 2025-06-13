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
#include <signal/signal.h>

namespace Cashmere
{

struct Entry;
struct ClockEntry;

using ClockEntryMap = std::map<Clock, Entry>;
using ReplaceEntryMap = std::map<Clock, ClockEntry>;
using ClockEntryList = std::list<ClockEntry>;
using Port = int64_t;

struct Entry
{
  Id id;
  Amount value;
  Clock alters;
  const bool operator==(const Entry& other) const;
  bool valid() const;
};

struct ClockEntry
{
  Clock clock;
  Entry entry;
  const bool operator==(const ClockEntry& other) const;
};

}

#endif
