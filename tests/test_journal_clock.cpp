#include "journal.h"
#include <catch2/catch_all.hpp>

SCENARIO("journal clock behaviour")
{
  GIVEN("an empty journal object")
  {
    Cashmere::Journal journal;
    THEN("the journal vector clock shows a single entry")
    {
      REQUIRE(journal.clock().size() == 1);
    }
  }
}
