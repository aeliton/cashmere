#include "journal.h"
#include <catch2/catch_all.hpp>

SCENARIO("journal id creation")
{
  GIVEN("an empty journal object")
  {
    Cashmere::Journal journal;
    THEN("the journal has a clock shows a single entry")
    {
      REQUIRE(journal.clock().size() == 1);
    }

    WHEN("creating a second journal")
    {
      Cashmere::Journal second;
      THEN("the journals belong to different pools")
      {
        REQUIRE(journal.bookId() != second.bookId());
      }
    }
  }
}
