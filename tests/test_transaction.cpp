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

using namespace Cashmere;

SCENARIO("adds and edits transactions")
{
  GIVEN("an empty ledger")
  {
    Ledger ledger;
    const Ledger::Id kId0 = ledger.id();
    THEN("the ledger won't have any transaction")
    {
      REQUIRE(ledger.transactions().size() == 0);
    }
    THEN("the ledger's clock value is zero")
    {
      REQUIRE(ledger.clock() == Ledger::Clock{{kId0, 0}});
    }
    WHEN("adding the first transaction")
    {
      constexpr uint64_t kTransactionValue = 500;
      const bool result = ledger.add(kTransactionValue);
      THEN("it must succeed")
      {
        REQUIRE(result);
      }
      AND_THEN("the transaction value matches the transaction added")
      {
        REQUIRE(ledger.query(1) == kTransactionValue);
      }
      AND_WHEN("adding a transaction with another ID")
      {
        constexpr Ledger::Id kId1 = 0xbeeffeed;
        ledger.add(kId1, Time(2), Amount(900));
        THEN("the transaction is retrievable")
        {
          REQUIRE(ledger.query(kId1, 2) == 900);
        }
        AND_THEN("the clock reports the current times for all ledger ids")
        {
          REQUIRE(ledger.clock() == Ledger::Clock{{kId0, 1UL}, {kId1, 2UL}});
        }
      }
    }
  }
}
