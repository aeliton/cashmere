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
    constexpr Journal::Id kId_FF = 0xFF;
    auto journal = std::make_shared<Journal>();
    journal->append(kId_FF, 300);
    journal->append(kId_FF, 200);
    journal->append(kId_FF, 100);

    const Journal::Clock kReplaceClock = {{kId_FF, 1}, {journal->id(), 0}};

    REQUIRE(journal->query({{kId_FF, 1}, {journal->id(), 0}}).value == 300);
    REQUIRE(
        journal->clock() == Journal::Clock{{kId_FF, 3}, {journal->id(), 0}});

    WHEN("using a ledger to process the journal")
    {
      Ledger ledger(journal);
      THEN("the recorded journal entries are consolidated")
      {
        REQUIRE(ledger.balance() == 600);
      }
      AND_WHEN("editing one of the entries")
      {
        constexpr Journal::Id kId_AA = 0xAA;
        journal->append(
            kId_AA, {Journal::Operation::Replace, 50, kReplaceClock});
        THEN("the balance is updated accordingly")
        {
          REQUIRE(ledger.balance() == 350);
          AND_THEN("the clock has increased after the replace call")
          {
            REQUIRE(journal->clock() == Journal::Clock{{kId_FF, 3}, {kId_AA, 1},
                                            {journal->id(), 0}});
          }
          AND_WHEN("editing the same entry a second time")
          {
            journal->append(
                kId_AA, {Journal::Operation::Replace, 25, kReplaceClock});
            THEN("the second edit takes priority over the previous")
            {
              REQUIRE(ledger.balance() == 325);
              AND_THEN("the clock is updated accordingly")
              {
                REQUIRE(
                    journal->clock() == Journal::Clock{{kId_FF, 3}, {kId_AA, 2},
                                            {journal->id(), 0}});
              }
            }
          }
        }
        AND_WHEN("deleting another entry")
        {
          journal->erase(
              kId_FF, Journal::Clock{{kId_FF, 2}, {journal->id(), 0}});
          THEN("the balance is updated accordingly")
          {
            REQUIRE(ledger.balance() == 150);
          }
        }
      }
    }
  }
}
