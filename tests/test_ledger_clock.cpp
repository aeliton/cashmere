#include "ledger.h"
#include <catch2/catch_all.hpp>

SCENARIO("ledger clock behaviour")
{
  GIVEN("an empty ledger object")
  {
    Cashmere::Ledger ledger;
    THEN("the ledger vector clock shows a single entry")
    {
      REQUIRE(ledger.clock().size() == 1);
    }
  }
}
