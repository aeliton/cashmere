#include "journal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

using Clock = Journal::Clock;

SCENARIO("journal clock behaviour")
{
  GIVEN("an empty journal object")
  {
    Journal journal;
    THEN("the journal vector clock shows a single entry")
    {
      REQUIRE(journal.clock() == Clock{{journal.id(), 0}});
    }
    WHEN("query for an inexisting transaction")
    {
      THEN("the invalid entry is returned")
      {
        REQUIRE_FALSE(journal.query(Clock{{journal.id(), 0}}).valid());
      }
    }
  }
}
