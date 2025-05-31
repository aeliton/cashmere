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

SCENARIO("the broker listens to journal transactions")
{
  GIVEN("an empty journal")
  {
    auto journal = std::make_shared<Journal>(0xAA);
    Broker broker;
    broker.attach(journal);

    REQUIRE(broker.versions() == std::map<Id, Clock>{});

    WHEN("the journal adds an entry")
    {
      journal->append(10);

      THEN("the broker has the updated clock version of the journal")
      {
        REQUIRE(broker.versions() == std::map<Id, Clock>{{0xAA, {{0xAA, 1}}}});
      }
    }
  }

  GIVEN("journal with pre-existing transactions")
  {
    auto journal = std::make_shared<Journal>(0xAA);
    journal->append(10);

    Broker broker;
    broker.attach(journal);

    THEN("the broker initializes the version of the journal")
    {
      REQUIRE(
          broker.versions() == std::map<Id, Clock>{{0xAA, Clock{{0xAA, 1}}}});
    }
  }
}

SCENARIO("a broker with multiple journals attached")
{
  Broker broker;
  GIVEN("two journals")
  {
    const auto a = std::make_shared<Journal>(0xAA);
    const auto b = std::make_shared<Journal>(0xBB);
    broker.attach(a);
    broker.attach(b);
    WHEN("one journal adds an entry")
    {
      a->append(10);
      THEN("the other journal gets the transaction via the broker")
      {
        REQUIRE(b->clock() == Clock{{0xAA, 1}});
        AND_THEN("the entry is retrievable the second journal")
        {
          REQUIRE(b->query({{0xAA, 1}}) == Journal::Entry{0xAA, 10, {}});
        }
      }
    }
  }
}
