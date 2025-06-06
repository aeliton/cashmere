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
#include "entry.h"

namespace Cashmere
{

EntryHandler::~EntryHandler() = default;

const bool ClockEntry::operator==(const ClockEntry& other) const
{
  return clock == other.clock && entry == other.entry;
}

EntryHandler::EntryHandler(Id id)
  : _id(id)
{
}

Id EntryHandler::id() const
{
  return _id;
}

ClockChangeSignal& EntryHandler::clockChanged()
{
  return _clockChanged;
}

void EntryHandler::setClock(const Clock& clock)
{
  _clock = clock;
}

void EntryHandler::clockTick(Id id)
{
  _clock[id]++;
};

Clock EntryHandler::clock() const
{
  return _clock;
};

}
