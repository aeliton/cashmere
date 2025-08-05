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
#include <catch2/catch_all.hpp>

using namespace Cashmere;

TEST_CASE("incomparable clocks")
{
  REQUIRE(Clock{{0xAA, 1}}.concurrent(Clock{{0xBB, 1}}));
  REQUIRE(Clock{{0xAA, 2}, {0xBB, 2}}.concurrent(Clock{{0xCC, 1}}));
}

TEST_CASE("comparable clocks", "[clock]")
{
  REQUIRE(Clock{{0xFF, 2}}.smallerThan(Clock{{0xAA, 1}, {0xFF, 2}}));
  REQUIRE_FALSE(Clock{{0xAA, 1}}.concurrent(Clock{{0xAA, 1}}));
  REQUIRE_FALSE(Clock{{0xAA, 1}}.smallerThan(Clock{{0xAA, 1}}));
}

TEST_CASE("from string", "[clock]")
{
  auto [pattern, expected] = GENERATE(table<std::string, Clock>(
    {{"{}", Clock{}},                  // empty no spaces
     {" {}", Clock{}},                 // spaces before the first open bracket
     {"{ }", Clock{}},                 // spaces after the first open bracket
     {"{{AA, 1}}", Clock{{0xAA, 1}}},  // well formated single entry clock
     {"{{ AA, 1}}", Clock{{0xAA, 1}}}, // ' ' after id-count pair open bracket
     {"{{AA , 1}}", Clock{{0xAA, 1}}}, // spaces after id pair bracket
     {"{{AA,  1}}", Clock{{0xAA, 1}}}, // multiple spaces after comma
     {"{{AA, 1 }}", Clock{{0xAA, 1}}}, // space after count
     {"{{AA, 1} }", Clock{{0xAA, 1}}}, // ' ' after id-count pair close bracket
     {" { { AA ,  1} ,  { BB , 1 } }", Clock{{0xAA, 1}, {0xBB, 1}}}}
  ));
  std::istringstream ins(pattern);
  Clock clock;
  CAPTURE(pattern);
  REQUIRE(Clock::Read(ins, clock));
  REQUIRE(clock == expected);
}

TEST_CASE("from string with invalid ids and times ", "[clock]")
{
  auto pattern =
    GENERATE("{{zz, 1}}", "{{ AA, aa}}", "", "{{AA, 1}", "{AA, 1}");
  std::istringstream ins(pattern);
  Clock clock;
  CAPTURE(pattern);
  REQUIRE_FALSE(Clock::Read(ins, clock));
}
