#include "journal.h"
#include <catch2/catch_all.hpp>

SCENARIO("journal id creation")
{
  GIVEN("an empty journal object")
  {
    Cashmere::Journal journal;
    THEN("the journal has an empty clock")
    {
      REQUIRE(journal.clock().empty());
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
