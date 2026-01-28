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
#ifndef CASHMERE_TYPES_H
#define CASHMERE_TYPES_H

#include <cashmere/cashmere_export.h>
#include <cstdint>
#include <set>

namespace Cashmere
{
using Amount = int64_t;
using Id = uint64_t;
using Time = uint64_t;
using Source = uint32_t;

using IdSet = std::set<Id>;

}

#endif
