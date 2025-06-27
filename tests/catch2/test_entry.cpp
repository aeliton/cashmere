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
#include <catch2/catch_all.hpp>

using namespace Cashmere;

TEST_CASE("from string", "[Data]")
{
  Data data;
  SECTION("valid empty data")
  {
    std::istringstream ins("{0, 0, {}}");
    REQUIRE(Data::Read(ins, data));
    REQUIRE(data == Data{});
  }
  SECTION("valid non-empty data with empty clock")
  {
    std::istringstream ins("{aa, 10, {}}");
    REQUIRE(Data::Read(ins, data));
    REQUIRE(data == Data{0xAA, 10, {}});
  }
}

TEST_CASE("from string", "[entry]")
{
  Entry data;
  SECTION("valid empty entry")
  {
    std::istringstream ins("{{}, {0, 0, {}}}");
    REQUIRE(Entry::Read(ins, data));
    REQUIRE(data == Entry{});
  }
  SECTION("valid non-empty entry with")
  {
    std::istringstream ins("{{{aa, 1}}, {aa, 10, {}}}");
    REQUIRE(Entry::Read(ins, data));
    REQUIRE(data == Entry{{{0xAA, 1}}, {0xAA, 10, {}}});
  }
}
