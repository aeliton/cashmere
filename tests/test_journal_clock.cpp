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
    WHEN("adding a entry with the journal ID")
    {
      journal.append(1000);
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{journal.id(), 1}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xbaadcafe, 200);
        THEN("the clock is updated")
        {
          REQUIRE(journal.clock() == Clock{{journal.id(), 1}, {0xbaadcafe, 1}});
        }
      }
    }
  }
}
