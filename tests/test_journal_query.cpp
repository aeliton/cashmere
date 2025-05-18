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

using Clock = Journal::Clock;
using Operation = Journal::Operation;
using Entry = Journal::Entry;

SCENARIO("journal queries")
{
  GIVEN("an empty journal object")
  {
    Journal journal;

    WHEN("query for an inexisting transaction")
    {
      THEN("the invalid entry is returned")
      {
        REQUIRE_FALSE(journal.query(Clock{{journal.id(), 0}}).valid());
      }
    }

    WHEN("adding an entry")
    {
      journal.append(1000);
      THEN("the entry is retrievable")
      {
        const auto clock = Clock{{journal.id(), 1}};
        REQUIRE(journal.query(clock) == Entry{Operation::Insert, 1000, {}});
      }

      THEN("query ignores zero values in clock entries")
      {
        const auto clock =
            Clock{{journal.id(), 1}, {0xbeef, 0}, {0xbaad, 0}, {0xcafe, 0}};
        REQUIRE(journal.query(clock) == Entry{Operation::Insert, 1000, {}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xbaadcafe, 200);
        THEN("the query with the updated clock returns the new transaction")
        {
          const auto clock = Clock{{journal.id(), 1}, {0xbaadcafe, 1}};
          REQUIRE(journal.query(clock) == Entry{Operation::Insert, 200, {}});
        }
      }
    }
  }
}
