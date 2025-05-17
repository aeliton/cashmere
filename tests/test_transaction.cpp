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
#include "journal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

using Clock = Journal::Clock;

SCENARIO("adds and edits transactions")
{
  GIVEN("an empty journal")
  {
    Journal journal;
    const Journal::Id kId0 = journal.id();
    THEN("the journal won't have any transaction")
    {
      REQUIRE(journal.journals().size() == 0);
    }
    THEN("the journal's clock value is zero")
    {
      REQUIRE(journal.clock() == Clock{{kId0, 0}});
    }

    WHEN("adding the first transaction")
    {
      constexpr uint64_t kTransactionValue = 500;
      const bool result = journal.append(kTransactionValue);
      THEN("it must succeed")
      {
        REQUIRE(result);
      }
      AND_THEN("the transaction value matches the transaction added")
      {
        REQUIRE(journal.query(Clock{{kId0, 1UL}}).value == kTransactionValue);
      }
      AND_WHEN("adding a second transaction")
      {
        journal.append(200);
        THEN("the second transaction is retrievable")
        {
          REQUIRE(journal.query(Clock{{kId0, 2}}).value == 200);
        }
      }

      WHEN("adding a transaction with another ID")
      {
        constexpr Journal::Id kId1 = 0xbeeffeed;
        journal.append(kId1, 900);
        THEN("the transaction is retrievable")
        {
          REQUIRE(journal.query().value == 900);
        }
        AND_THEN("the clock reports the current times for all journal ids")
        {
          REQUIRE(journal.query(Clock{{kId0, 1UL}, {kId1, 1UL}}).value == 900);
        }
      }
    }
  }
}
