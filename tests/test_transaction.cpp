// Cashmere - a distributed conflict-free replicated database.
// Copyright (C) 2025 Aeliton G. Silva
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include "ledger.h"
#include <catch2/catch_all.hpp>

SCENARIO("adds and edits transactions")
{
  GIVEN("an empty ledger")
  {
    Cashmere::Ledger ledger;
    THEN("the ledger won't have any transaction")
    {
      REQUIRE(ledger.transactions().size() == 0);
    }
    THEN("the ledger's clock value is zero")
    {
      REQUIRE(ledger.clock() == Cashmere::Clock{{ledger.id(), 0}});
    }
    WHEN("adding the first transaction")
    {
      constexpr uint64_t kTransactionValue = 500;
      const bool result = ledger.add(kTransactionValue);
      THEN("it must succeed")
      {
        REQUIRE(result);
      }
      AND_WHEN("retrieving the transactions")
      {
        auto [clock, transaction] = *ledger.transactions().begin();
        THEN("the transaction value matches the transaction added")
        {
          REQUIRE(transaction.value == kTransactionValue);
        }
        AND_THEN("the ledger clock is incremented")
        {
          REQUIRE(clock == Cashmere::Clock{{ledger.id(), 1}});
        }
      }
    }
  }
}
