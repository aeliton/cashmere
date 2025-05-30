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
    auto journal = std::make_shared<Journal>();
    Broker broker(journal);

    REQUIRE(broker.presense() == std::map<Id, Clock>{});

    WHEN("the journal adds an entry")
    {
      journal->append(10);

      THEN("the broker has the updated clock version of the journal")
      {
        REQUIRE(broker.presense() == std::map<Id, Clock>{{journal->id(),
                                         Clock{{journal->id(), 1}}}});
      }
    }
  }

  GIVEN("journal with pre-existing transactions")
  {
    auto journal = std::make_shared<Journal>();
    journal->append(10);

    Broker broker(journal);
    THEN("the broker initializes the version of the journal")
    {
      REQUIRE(broker.presense() ==
              std::map<Id, Clock>{{journal->id(), Clock{{journal->id(), 1}}}});
    }
  }
}
