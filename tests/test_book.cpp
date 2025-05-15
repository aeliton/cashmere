#include "ledger.h"
#include <catch2/catch_all.hpp>

SCENARIO("within book transactions")
{
  GIVEN("an empty ledger")
  {
    Cashmere::Journal a;

    WHEN("creating a second ledger passing the a book ID")
    {
      Cashmere::Journal b(a.bookId());
      THEN("the second ledger has the same poolId as the first")
      {
        REQUIRE(b.bookId() == a.bookId());
      }
    }
  }
}
