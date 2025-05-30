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
    Journal journal;
    THEN("the journal vector clock is empty")
    {
      REQUIRE(journal.clock() == Clock{});
    }
    WHEN("adding a entry with the journal ID")
    {
      journal.append(1000);
      const auto kId = journal.id();
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{kId, 1}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xAA, 200);
        THEN("the clock is updated")
        {
          REQUIRE(journal.clock() == Clock{{kId, 1}, {0xAA, 1}});
        }
        AND_WHEN("another journal replaces a transaction")
        {
          journal.replace(0xFF, 300, {{kId, 1}});
          THEN("the actor's id is incremented")
          {
            REQUIRE(journal.clock() == Clock{{kId, 1}, {0xAA, 1}, {0xFF, 1}});
          }
          AND_WHEN("an entry is deleted")
          {
            journal.erase({{kId, 1}});
            THEN("the actor's is incremented")
            {
              REQUIRE(journal.clock() == Clock{{kId, 2}, {0xAA, 1}, {0xFF, 1}});
            }
          }
        }
      }
    }
  }
}

SCENARIO("zero values in clocks are ignored")
{
  GIVEN("a journal with a transaction")
  {
    Journal journal;
    const auto id = journal.id();

    journal.append(1000);

    THEN("querying the transaction suceeds")
    {
      REQUIRE(journal.query({{id, 1}}).value == 1000);
    }

    const auto validClockWithZeroes = Clock{{id, 1}, {0xbaad, 0}, {0xcafe, 0}};

    WHEN("querying with extra ids with count zero")
    {
      const auto result = journal.query(validClockWithZeroes);
      THEN("the clock's zeroed entries are ignored")
      {
        REQUIRE(result == Journal::Entry{id, 1000, {}});
      }
    }

    WHEN("replacing a transaction using a clock with zeroed id counts")
    {
      const bool success = journal.replace(500, validClockWithZeroes);

      REQUIRE(success);

      AND_WHEN("querying the recorded entry")
      {
        const auto result = journal.query({{id, 2}});
        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Journal::Entry{id, 500, {{id, 1}}});
        }
      }
    }

    WHEN("inserting a transaction using a clock with zeroed id counts")
    {

      WHEN("a conflicting transaction is received")
      {
        journal.insert({{0xAA, 0}, {0xBB, 0}, {0xCC, 1}}, {id, 206, {}});
        AND_WHEN("querying the inserted entry")
        {
          const auto result = journal.query({{0xCC, 1}});
          THEN("the zeroed entries are ignored")
          {
            REQUIRE(result == Journal::Entry{id, 206, {}});
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
        const auto result = journal.query({{id, 2}});
        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Journal::Entry{id, 0, {{id, 1}}});
        }
      }
    }
  }
}

SCENARIO("concurrent transactions update clock")
{
  GIVEN("a journal with a transaction")
  {
    Journal journal;
    journal.append(10);
    WHEN("a conflicting transaction is received")
    {
      journal.insert(
          {{journal.id(), 1}, {0xAA, 1}}, {0xAA, 20, {{journal.id(), 1}}});
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{journal.id(), 1}, {0xAA, 1}});
      }
    }
  }
}
