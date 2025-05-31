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
#include "journal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

using Entry = Journal::Entry;

SCENARIO("journal queries")
{
  GIVEN("an empty journal object")
  {
    Journal journal(0xAA);

    WHEN("query for an inexisting transaction")
    {
      THEN("the invalid entry is returned")
      {
        REQUIRE_FALSE(journal.query(Clock{{0xAA, 0}}).valid());
      }
    }

    WHEN("adding an entry")
    {
      journal.append(1000);
      const Clock firstEntryClock = {{0xAA, 1}};
      THEN("the entry is retrievable")
      {
        REQUIRE(journal.query(firstEntryClock) == Entry{0xAA, 1000, {}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xbaadcafe, 200);
        THEN("the query with the updated clock returns the new transaction")
        {
          const auto clock = Clock{{0xAA, 1}, {0xbaadcafe, 1}};
          REQUIRE(journal.query(clock) == Entry{0xbaadcafe, 200, {}});
        }
      }
    }
  }
}
