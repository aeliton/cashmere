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
#include "cashmere/clock.h"
#include "cashmere/utils/file.h"

#include <istream>
#include <sstream>

namespace Cashmere
{

Clock::Clock()
  : std::map<Id, Time>()
{
}

Clock::Clock(const std::initializer_list<std::pair<const Id, Time>>& list)
  : std::map<Id, Time>(list)
{
  std::erase_if(*this, [](const auto& item) {
    return item.first != 0 && item.second == 0;
  });
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
  os << "{" << std::hex << it->first << std::dec << ", " << it->second << "}";
  for (++it; it != clock.end(); it++) {
    os << ", {" << std::hex << it->first << std::dec << ", " << it->second
       << "}";
  }
  os << "}";
  return os;
}

bool Clock::valid() const
{
  return find(0) == cend();
}

bool Clock::isNext(const Clock& other, Id id) const
{
  const auto it = find(id);
  if (it == cend()) {
    return false;
  }
  if (other.find(id) == other.cend()) {
    return it->second == 1;
  } else {
    return at(id) == other.at(id) + 1;
  }
}

bool Clock::Read(std::istream& in, Clock& clock)
{
  if (!ReadChar(in, kOpenCurly)) {
    return false;
  }

  ReadSpaces(in);
  if (in.peek() == kOpenCurly) {
    do {
      Id id;
      Time time;
      if (!ReadPair(in, id, time)) {
        return false;
      }
      clock[id] = time;
    } while (ReadChar(in, kComma));
  }

  if (!ReadChar(in, kCloseCurly)) {
    return false;
  }

  return true;
}

std::string Clock::str() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

}
