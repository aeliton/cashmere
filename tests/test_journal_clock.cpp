#include "journal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

using Clock = Journal::Clock;
using Operation = Journal::Operation;

SCENARIO("journal clock behaviour")
{
  GIVEN("an empty journal object")
  {
    Journal journal;
    THEN("the journal vector clock is empty")
    {
      REQUIRE(journal.clock() == Clock{});
    }
    WHEN("query for an inexisting transaction")
    {
      THEN("the invalid entry is returned")
      {
        REQUIRE_FALSE(journal.query(Clock{{journal.id(), 0}}).valid());
      }
    }

    WHEN("adding a entry with the journal ID")
    {
      journal.append(1000);
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{journal.id(), 1}});
      }
      AND_THEN("the entry is retrievable")
      {
        REQUIRE(journal.query(Clock{{journal.id(), 1}}) ==
                Journal::Entry{Operation::Insert, 1000, {}});
      }
      AND_THEN("the entry is also retrievable if providing extra ids with time "
               "zeroed")
      {
        auto clock =
            Clock{{journal.id(), 1}, {0xbeef, 0}, {0xbaad, 0}, {0xcafe, 0}};
        REQUIRE(journal.query(clock) ==
                Journal::Entry{Operation::Insert, 1000, {}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xbaadcafe, 200);
        THEN("the clock is updated")
        {
          REQUIRE(journal.clock() == Clock{{journal.id(), 1}, {0xbaadcafe, 1}});
        }
        AND_THEN("the query with the updated clock returns the new transaction")
        {
          REQUIRE(journal.query(Clock{{journal.id(), 1}, {0xbaadcafe, 1}}) ==
                  Journal::Entry{Operation::Insert, 200, {}});
        }
      }
    }
  }
}
