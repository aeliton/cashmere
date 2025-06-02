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

SCENARIO("journal clock updates when entries are changed")
{
  GIVEN("an empty journal object")
  {
    auto journal = std::make_shared<Journal>(0xAA);

    THEN("the journal vector clock is empty")
    {
      REQUIRE(journal->clock() == Clock{});
    }

    WHEN("query for an inexisting transaction")
    {
      THEN("the invalid entry is returned")
      {
        REQUIRE_FALSE(journal->query(Clock{{0xAA, 0}}).valid());
      }
    }

    WHEN("appending an entry")
    {
      journal->append(1000);
      THEN("the clock is updated")
      {
        REQUIRE(journal->clock() == Clock{{0xAA, 1}});
        AND_THEN("the entry is queryable")
        {
          REQUIRE(journal->query({{0xAA, 1}}) == Entry{0xAA, 1000, {}});
        }
      }

      WHEN("inserting a transaction using an existing clock")
      {
        const bool success = journal->insert({{0xAA, 1}}, {0xAA, -1, {}});
        THEN("it should fail")
        {
          REQUIRE_FALSE(success);
        }
      }

      AND_WHEN("appending a second entry")
      {
        journal->append(333);
        THEN("the clock is updated")
        {
          REQUIRE(journal->clock() == Clock{{0xAA, 2}});
        }
      }

      AND_WHEN("inserting an entry from another journal")
      {
        journal->insert({{0xFF, 1}}, {0xFF, 200, {}});
        THEN("the clock is updated to show the incoming entry")
        {
          REQUIRE(journal->clock() == Clock{{0xAA, 1}, {0xFF, 1}});
        }
      }
    }
  }
}

SCENARIO("zero values in clocks are ignored")
{
  GIVEN("a journal with a transaction")
  {
    Journal journal(0xAA);

    journal.append(1000);

    THEN("querying the transaction suceeds")
    {
      REQUIRE(journal.query({{0xAA, 1}}).value == 1000);
    }

    const auto validClockWithZeroes =
        Clock{{0xAA, 1}, {0xbaad, 0}, {0xcafe, 0}};

    WHEN("querying with extra ids with count zero")
    {
      const auto result = journal.query(validClockWithZeroes);
      THEN("the clock's zeroed entries are ignored")
      {
        REQUIRE(result == Entry{0xAA, 1000, {}});
      }
    }

    WHEN("replacing a transaction using a clock with zeroed id counts")
    {
      const bool success = journal.replace(500, validClockWithZeroes);

      REQUIRE(success);

      AND_WHEN("querying the recorded entry")
      {
        const auto result = journal.query({{0xAA, 2}});
        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Entry{0xAA, 500, {{0xAA, 1}}});
        }
      }
    }

    WHEN("inserting a transaction using a clock with zeroed id counts")
    {

      WHEN("a conflicting transaction is received")
      {
        journal.insert({{0xAA, 0}, {0xBB, 0}, {0xCC, 1}}, {0xAA, 206, {}});
        AND_WHEN("querying the inserted entry")
        {
          const auto result = journal.query({{0xCC, 1}});
          THEN("the zeroed entries are ignored")
          {
            REQUIRE(result == Entry{0xAA, 206, {}});
          }
        }
      }
    }

    WHEN("erasing a transaction using a clock with zeroed id counts")
    {
      const bool success = journal.erase(validClockWithZeroes);

      REQUIRE(success);

      AND_WHEN("querying the recorded entry")
      {
        const auto result = journal.query({{0xAA, 2}});
        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Entry{0xAA, 0, {{0xAA, 1}}});
        }
      }
    }
  }
}
