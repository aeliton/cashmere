#include "journal.h"
#include <catch2/catch_all.hpp>

SCENARIO("within book transactions")
{
  GIVEN("an empty journal")
  {
    Cashmere::Journal a;

    WHEN("creating a second journal passing the a book ID")
    {
      Cashmere::Journal b(a.bookId());
      THEN("the second journal has the same poolId as the first")
      {
        REQUIRE(b.bookId() == a.bookId());
      }
    }
  }
}
