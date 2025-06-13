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
#include "clock.h"

namespace Cashmere
{

Clock::Clock()
  : std::map<Id, Time>()
{
}

Clock::Clock(const std::initializer_list<std::pair<const Id, Time>>& list)
  : std::map<Id, Time>(list)
{
  std::erase_if(*this, [](const auto& item) { return item.second == 0; });
}

Clock Clock::merge(const Clock& other) const
{
  Clock out = *this;
  for (auto& [id, count] : other) {
    out[id] = std::max(find(id) != end() ? at(id) : 0, count);
  }
  return out;
}

Clock Clock::tick(Id id) const
{
  Clock out = *this;
  ++out[id];
  return out;
}

bool Clock::smallerThan(const Clock& other) const
{
  return *this != other && merge(other) == other;
}

bool Clock::concurrent(const Clock& other) const
{
  return *this != other && !smallerThan(other) && !other.smallerThan(*this);
}

std::ostream& operator<<(std::ostream& os, const Clock& clock)
{
  os << "{";
  if (clock.empty()) {
    os << "}";
    return os;
  }

  auto it = clock.begin();
  os << "{" << it->first << ", " << it->second << "}";
  it++;
  for (; it != clock.end(); it++) {
    os << ", {" << it->first << ", " << it->second << "}";
  }
  os << "}";
  return os;
}
}
