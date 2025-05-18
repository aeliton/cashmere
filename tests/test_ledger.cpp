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

using Clock = Journal::Clock;

SCENARIO("a ledger handles entries of multiple nodes")
{
  GIVEN("a journal with a few transactions")
  {
    constexpr Journal::Id kId_FF = 0xFF;
    auto journal = std::make_shared<Journal>();
    journal->append(kId_FF, 300);
    journal->append(kId_FF, 200);
    journal->append(kId_FF, 100);

    WHEN("a ledger to process the journal")
    {
      Ledger ledger(journal);
      THEN("the recorded journal entries are consolidated")
      {
        REQUIRE(ledger.balance() == 600);
      }
      AND_WHEN("editing one of the entries")
      {
        constexpr Journal::Id kId_AA = 0xAA;
        journal->replace(kId_AA, 50, Clock{{kId_FF, 1}});

        THEN("the balance is updated accordingly")
        {
          REQUIRE(ledger.balance() == 350);

          AND_WHEN("the same node edits the same entry")
          {
            journal->replace(kId_AA, 25, Clock{{kId_FF, 1}});
            THEN("the second edit takes priority over the previous")
            {
              REQUIRE(ledger.balance() == 325);
            }
          }
          AND_WHEN("editing the same entry by another journal")
          {
            journal->replace(kId_FF, 10, Clock{{kId_FF, 1}});
            THEN("the edit from greatest journal id takes priority")
            {
              REQUIRE(ledger.balance() == 310);
            }
          }
        }
        AND_WHEN("deleting another entry")
        {
          journal->erase(kId_FF, Clock{{kId_FF, 2}});
          THEN("the balance is updated accordingly")
          {
            REQUIRE(ledger.balance() == 150);
          }
        }
      }
    }
  }
}
