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
#include <istream>

namespace Cashmere
{

bool Entry::operator==(const Entry& other) const
{
  return clock == other.clock && entry == other.entry;
}

std::ostream& operator<<(std::ostream& os, const Data& data)
{
  return os << "{" << std::hex << data.id << std::dec << ", " << data.value
            << ", " << data.alters << "}";
}

std::ostream& operator<<(std::ostream& os, const Entry& data)
{
  return os << "{" << data.clock << ", " << data.entry << "}";
}

bool Data::valid() const
{
  return alters.size() > 0 && alters.begin()->first != 0UL;
}

bool Data::operator==(const Data& other) const
{
  return std::tie(id, value, alters) ==
         std::tie(other.id, other.value, other.alters);
}

bool Data::Read(std::istream& in, Data& data)
{
  int c;
  if ((c = in.peek()) == ' ') {
    in.get();
  }
  if ((c = in.get()) != '{') {
    return false;
  }
  if (in.peek() == '}') {
    data = {};
    return true;
  }
  Id id;
  in >> std::hex >> id >> std::dec;
  in.get();

  Amount amount;
  in >> amount;
  in.get();

  Clock clock;
  if (!Clock::Read(in, clock)) {
    return false;
  }

  if ((c = in.get()) != '}') {
    return false;
  }

  data.id = id;
  data.value = amount;
  data.alters = clock;
  return true;
}

}
