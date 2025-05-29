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
#ifndef CASHMERE_CLOCK_H
#define CASHMERE_CLOCK_H

#include <map>

#include "cashmere.h"

namespace Cashmere
{

class Clock : public std::map<Id, Time>
{
public:
  Clock();
  Clock(const std::initializer_list<std::pair<const Id, Time>>& list);
  Clock merge(const Clock& other) const;
  bool smallerThan(const Clock& other) const;
  bool concurrent(const Clock& other) const;
};

}
#endif
