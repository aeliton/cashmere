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
    THEN("the ledger won't have any transaction")
    {
      REQUIRE(ledger.transactions().size() == 0);
    }
    THEN("the ledger's clock value is zero")
    {
      REQUIRE(ledger.clock() == Clock{{ledger.id(), 0}});
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
        const auto transactions = ledger.transactions().at(ledger.id());
        THEN("the transaction value matches the transaction added")
        {
          REQUIRE(transactions.at(Time(1)) == kTransactionValue);
        }
      }
      AND_WHEN("adding a transaction with another ID")
      {
        constexpr Id kSecondId = 0xbeeffeed;
        ledger.add(kSecondId, Time(2), Amount(900));
        THEN("the transaction is retrievable")
        {
          REQUIRE(ledger.transactions().at(kSecondId).at(Time(2)) == 900);
        }
      }
    }
  }
}
