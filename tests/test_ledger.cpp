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

SCENARIO("ledger process two alternating edits of the same transaction")
{
  GIVEN("a ledger processing a journal with a single entry initially")
  {
    constexpr Journal::Id kTinyId = 0xAA;
    constexpr Journal::Id kHugeId = 0xFF;

    auto journal = std::make_shared<Journal>();
    journal->append(kHugeId, 100);

    const Clock toReplaceClock = {{kHugeId, 1}};

    REQUIRE(journal->clock() == toReplaceClock);
    REQUIRE(journal->query(toReplaceClock).value == 100);

    Ledger ledger(journal);

    WHEN("a journal with a bigger ID replaces the entry")
    {
      journal->replace(kHugeId, 200, toReplaceClock);

      REQUIRE(journal->clock() == Clock{{kHugeId, 2}});
      REQUIRE(journal->query({{kHugeId, 2}}).value == 200);

      AND_WHEN("the journal with smaller ID replaces the same entry")
      {
        journal->replace(kTinyId, 300, toReplaceClock);
        REQUIRE(journal->query({{kTinyId, 1}, {kHugeId, 2}}).value == 300);

        THEN("the second replace takes precedence as it has a bigger time")
        {
          REQUIRE(ledger.balance() == 300);
        }
      }
    }
  }
}

SCENARIO("ledger process concurrent transactions")
{
  GIVEN("a ledger processing a journal with a single entry initially")
  {
    constexpr Journal::Id kAA = 0xAA;
    constexpr Journal::Id kBB = 0xBB;

    auto journal = std::make_shared<Journal>();
    journal->append(0xBB, 1);

    const Clock fixMe = {{0xBB, 1}};

    Ledger ledger(journal);

    WHEN("a journal with a bigger ID replaces the entry")
    {
      journal->replace(0xBB, 10, fixMe);

      AND_WHEN("the journal with smaller ID replaces the same entry")
      {
        journal->insert(0xAA, {{0xAA, 1}, {0xBB, 1}}, {0xAA, 100, fixMe});

        THEN("device with bigger ID takes precedence in concurrent edits")
        {
          REQUIRE(ledger.balance() == 10);
        }
      }
      AND_WHEN("another journal with a bigger id also conflicts")
      {
        journal->insert(0xCC, {{0xAA, 1}, {0xCC, 1}}, {0xCC, 1000, fixMe});
        THEN("device with bigger ID takes precedence in concurrent edits")
        {
          REQUIRE(ledger.balance() == 1000);
        }
      }
    }
  }
}
