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

SCENARIO("evaluate transactions")
{
  GIVEN("a journal with a few transactions")
  {
    auto journal = std::make_shared<Journal>();
    journal->append(300);
    journal->append(200);
    journal->append(100);

    REQUIRE(journal->query(1).value == 300);
    REQUIRE(journal->clock() == Journal::Clock{{journal->id(), 3}});

    WHEN("using a ledger to process the journal")
    {
      Ledger ledger(journal);
      THEN("the recorded journal entries are consolidated")
      {
        REQUIRE(ledger.balance() == 600);
      }
      AND_WHEN("editing one of the entries")
      {
        journal->replace(journal->id(), 1, 50);
        THEN("the balance is updated accordingly")
        {
          REQUIRE(ledger.balance() == 350);
          AND_THEN("the clock has increased after the replace call")
          {
            REQUIRE(journal->clock() == Journal::Clock{{journal->id(), 4}});
          }
          AND_WHEN("editing the same entry a second time")
          {
            journal->replace(journal->id(), 1, 25);
            THEN("the second edit takes priority over the previous")
            {
              REQUIRE(ledger.balance() == 325);
              AND_THEN("the clock is updated accordingly")
              {
                REQUIRE(journal->clock() == Journal::Clock{{journal->id(), 5}});
              }
            }
          }
        }
        AND_WHEN("deleting another entry")
        {
          journal->erase(journal->id(), 2);
          THEN("the balance is updated accordingly")
          {
            REQUIRE(ledger.balance() == 150);
          }
        }
      }
    }
  }
}
