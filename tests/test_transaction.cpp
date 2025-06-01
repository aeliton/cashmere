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
#include "broker.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

SCENARIO("adds and edits transactions")
{
  GIVEN("an empty journal")
  {
    constexpr Id kId0 = 0xbadcafe;
    constexpr Id kId1 = 0xbeeffeed;
    auto journal = std::make_shared<Journal>(kId0);
    auto second = std::make_shared<Journal>(kId1);

    Broker broker;
    broker.attach(journal);
    broker.attach(second);

    THEN("the journal won't have any transaction")
    {
      REQUIRE(journal->entries().size() == 0);
    }
    THEN("the journal's clock value is empty")
    {
      REQUIRE(journal->clock() == Clock{});
    }

    WHEN("adding the first transaction")
    {
      constexpr uint64_t kTransactionValue = 500;
      const bool result = journal->append(kTransactionValue);
      THEN("it must succeed")
      {
        REQUIRE(result);
      }
      AND_THEN("the transaction value matches the transaction added")
      {
        REQUIRE(journal->query(Clock{{kId0, 1UL}}).value == kTransactionValue);
      }
      AND_WHEN("adding a second transaction")
      {
        journal->append(200);
        THEN("the second transaction is retrievable")
        {
          REQUIRE(journal->query(Clock{{kId0, 2}}).value == 200);
        }
      }

      WHEN("adding a transaction with another ID")
      {
        second->append(900);
        THEN("the transaction is retrievable")
        {
          REQUIRE(journal->query(Clock{{kId0, 1UL}, {kId1, 1UL}}).value == 900);
        }
      }
    }
  }
}
