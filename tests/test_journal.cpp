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

TEST_CASE("clock starts empty for newly created journals", "[journal]")
{
  Journal journal;
  REQUIRE(journal.clock().empty());
}

TEST_CASE("invalid/inexisting queries returns invalid", "[journal]")
{
  SECTION("inexisting entry")
  {
    Journal journal;
    REQUIRE_FALSE(journal.entry(Clock{{0xAA, 1}}).valid());
  }

  SECTION("all zeroes clock")
  {
    Journal journal;
    REQUIRE_FALSE(journal.entry(Clock{{0xAA, 0}, {0xBB, 0}}).valid());
  }
}

SCENARIO("attempt inserting a transaction using an existing clock")
{
  GIVEN("an empty journal with a single transaction")
  {
    Journal journal(0xAA);
    journal.append(10);
    WHEN("the same clock is used to insert a transaction")
    {
      const auto data = ClockEntry{{{0xAA, 1}}, {0xAA, 10, {}}};
      const bool success = journal.insert(data);
      THEN("it should fail")
      {
        REQUIRE_FALSE(success);
      }
    }
  }
}

SCENARIO("journal clock updates when entries are changed")
{
  GIVEN("an empty journal object")
  {
    Journal journal(0xAA);

    WHEN("appending an entry")
    {
      journal.append(1000);
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{0xAA, 1}});
      }

      THEN("the entry is queryable")
      {
        REQUIRE(journal.entry(Clock{{0xAA, 1}}) == Entry{0xAA, 1000, {}});
      }

      AND_WHEN("appending a second entry")
      {
        journal.append(333);

        THEN("the clock is updated")
        {
          REQUIRE(journal.clock() == Clock{{0xAA, 2}});
        }

        THEN("the second entry is queryable")
        {
          REQUIRE(journal.entry(Clock{{0xAA, 2}}) == Entry{0xAA, 333, {}});
        }
      }

      AND_WHEN("inserting a second entry from another journal")
      {
        journal.insert(ClockEntry{{{0xFF, 1}}, {0xFF, 200, {}}});

        THEN("the clock is updated to show the incoming entry")
        {
          REQUIRE(journal.clock() == Clock{{0xAA, 1}, {0xFF, 1}});
        }

        THEN("the second entry is queryable")
        {
          REQUIRE(journal.entry(Clock{{0xFF, 1}}) == Entry{0xFF, 200, {}});
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
      REQUIRE(journal.entry({{0xAA, 1}}).value == 1000);
    }

    const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

    WHEN("querying with extra ids with count zero")
    {
      const auto result = journal.entry(validClockWithZeroes);

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
        const auto result = journal.entry({{0xAA, 2}});

        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Entry{0xAA, 500, {{0xAA, 1}}});
        }
      }
    }

    WHEN("inserting a transaction using a clock with zeroed id counts")
    {
      const auto clock = Clock{{0xAA, 0}, {0xBB, 0}, {0xCC, 1}};

      bool success = journal.insert(ClockEntry{clock, {0xAA, 206, {}}});

      THEN("it succeeds")
      {
        REQUIRE(success);
      }

      AND_WHEN("querying the inserted entry")
      {
        const auto result = journal.entry({{0xCC, 1}});

        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Entry{0xAA, 206, {}});
        }
      }
    }

    WHEN("erasing a transaction using a clock with zeroed id counts")
    {
      const bool success = journal.erase(validClockWithZeroes);

      REQUIRE(success);

      AND_WHEN("querying the recorded entry")
      {
        const auto result = journal.entry({{0xAA, 2}});
        THEN("the zeroed entries are ignored")
        {
          REQUIRE(result == Entry{0xAA, 0, {{0xAA, 1}}});
        }
      }
    }
  }
}

TEST_CASE("entries retrieval", "[entries]")
{
  const ClockEntryMap entries = {
    {Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}},
    {Clock{{0xBB, 1}}, Entry{0xBB, 10, {}}},
    {Clock{{0xAA, 2}, {0xBB, 1}}, Entry{0xAA, 2, Clock{{0xBB, 1}}}},
    {Clock{{0xCC, 1}}, Entry{0xCC, 100, {}}},
  };
  Journal journal(0xAA, entries);

  const ClockEntryList expected{{Clock{{0xCC, 1}}, Entry{0xCC, 100, {}}}};
  REQUIRE(journal.entries(Clock{{0xAA, 2}, {0xBB, 1}}) == expected);
}
