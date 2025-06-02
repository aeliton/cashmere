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
#include "ledger.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

SCENARIO("a ledger handles entries of multiple nodes")
{
  GIVEN("a journal with a few transactions")
  {
    constexpr Id kId_FF = 0xFF;
    auto journal = std::make_shared<Journal>(kId_FF);

    Broker broker;
    broker.attach(journal);

    journal->append(300);
    journal->append(200);
    journal->append(100);

    WHEN("a ledger to process the journal")
    {
      Ledger ledger(journal);
      THEN("the recorded journal entries are consolidated")
      {
        REQUIRE(ledger.balance() == 600);
      }
      AND_WHEN("editing one of the entries")
      {
        constexpr Id kId_AA = 0xAA;
        auto aa = std::make_shared<Journal>(kId_AA);

        broker.attach(aa);

        aa->replace(50, Clock{{kId_FF, 1}});

        THEN("the balance is updated accordingly")
        {
          REQUIRE(ledger.balance() == 350);

          AND_WHEN("the same node edits the same entry")
          {
            aa->replace(25, Clock{{kId_FF, 1}});
            THEN("the second edit takes priority over the previous")
            {
              REQUIRE(ledger.balance() == 325);
            }
          }
          AND_WHEN("editing the same entry by another journal")
          {
            journal->replace(10, Clock{{kId_FF, 1}});
            THEN("the edit from greatest journal id takes priority")
            {
              REQUIRE(ledger.balance() == 310);
            }
          }
        }
        AND_WHEN("deleting another entry")
        {
          journal->erase(Clock{{kId_FF, 2}});
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
    constexpr Id kTinyId = 0xAA;
    constexpr Id kHugeId = 0xFF;

    auto huge = std::make_shared<Journal>(kHugeId);
    auto tiny = std::make_shared<Journal>(kTinyId);

    Broker broker;
    broker.attach(huge);
    broker.attach(tiny);

    huge->append(100);

    const Clock toReplaceClock = {{kHugeId, 1}};

    REQUIRE(huge->clock() == toReplaceClock);
    REQUIRE(huge->query(toReplaceClock).value == 100);

    Ledger ledger(huge);

    WHEN("a journal with a bigger ID replaces the entry")
    {
      huge->replace(200, toReplaceClock);

      REQUIRE(huge->clock() == Clock{{kHugeId, 2}});
      REQUIRE(huge->query({{kHugeId, 2}}).value == 200);

      AND_WHEN("the journal with smaller ID replaces the same entry")
      {
        tiny->replace(300, toReplaceClock);
        REQUIRE(tiny->query({{kTinyId, 1}, {kHugeId, 2}}).value == 300);

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
    constexpr Id kAA = 0xAA;
    constexpr Id kBB = 0xBB;
    constexpr Id kCC = 0xCC;

    auto small = std::make_shared<Journal>(kAA);
    auto medium = std::make_shared<Journal>(kBB);
    auto big = std::make_shared<Journal>(kCC);

    Broker broker;
    broker.attach(medium);
    broker.attach(small);
    broker.attach(big);

    medium->append(1);

    const Clock fixMe = {{0xBB, 1}};

    Ledger ledgerSmall(small);
    Ledger ledgerMedium(medium);
    Ledger ledgerBig(big);

    WHEN("a journal with a bigger ID replaces the entry")
    {
      broker.detach(small->id());
      broker.detach(big->id());
      medium->replace(10, fixMe);

      AND_WHEN("the journal with smaller ID replaces the same entry")
      {
        small->replace(100, fixMe);
        REQUIRE(small->clock() == Clock{{kAA, 1}, {kBB, 1}});

        broker.attach(small);

        REQUIRE(small->clock() == Clock{{kAA, 1}, {kBB, 2}});
        REQUIRE(medium->clock() == Clock{{kAA, 1}, {kBB, 2}});
        REQUIRE(big->clock() == Clock{{kBB, 1}});

        THEN("device with bigger ID takes precedence in concurrent edits")
        {
          REQUIRE(ledgerSmall.balance() == 10);
          REQUIRE(ledgerMedium.balance() == 10);
          REQUIRE(ledgerBig.balance() == 1);
        }
      }
      AND_WHEN("another journal with a bigger id also conflicts")
      {
        broker.attach(big);
        big->replace(1000, fixMe);
        THEN("device with bigger ID takes precedence in concurrent edits")
        {
          REQUIRE(ledgerSmall.balance() == 1);
          REQUIRE(ledgerMedium.balance() == 1000);
          REQUIRE(ledgerBig.balance() == 1000);
        }
        AND_WHEN("attaching the last remaining offline journal")
        {
          broker.attach(small);
          THEN("all ledgers converge")
          {
            REQUIRE(ledgerSmall.balance() == 1000);
            REQUIRE(ledgerMedium.balance() == 1000);
            REQUIRE(ledgerBig.balance() == 1000);
          }
        }
      }
    }
  }
}
