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
#include "device.h"
#include <catch2/catch_all.hpp>

SCENARIO("device adds and edits transactions")
{
  GIVEN("an empty device object")
  {
    Cashmere::Device device;
    THEN("the device won't have any transaction")
    {
      REQUIRE(device.transactions().size() == 0);
    }
    WHEN("adding the first transaction")
    {
      constexpr uint64_t kTransactionValue = 500;
      const bool result = device.add(kTransactionValue);
      THEN("it must succeed")
      {
        REQUIRE(result);
      }
      AND_WHEN("retrieving the device's transactions")
      {
        THEN("the number of transactions has increased")
        {
          REQUIRE(device.transactions().size() == 1);
        }
        AND_THEN("the transaction value matches the transaction added")
        {
          auto [clock, transaction] = *device.transactions().begin();
          REQUIRE(transaction.value == kTransactionValue);
        }
      }
    }
  }
}
