#include "ledger.h"
#include <catch2/catch_all.hpp>

SCENARIO("ledger id creation")
{
  GIVEN("an empty ledger object")
  {
    Cashmere::Ledger ledger;
    THEN("the ledger has a clock shows a single entry")
    {
      REQUIRE(ledger.clock().size() == 1);
    }

    WHEN("creating a second ledger")
    {
      Cashmere::Ledger second;
      THEN("the ledgers belong to different pools")
      {
        REQUIRE(ledger.bookId() != second.bookId());
      }
    }
  }
}
