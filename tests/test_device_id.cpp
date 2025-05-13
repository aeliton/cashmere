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

SCENARIO("device id creation")
{
  GIVEN("an empty device object")
  {
    Cashmere::Device device;
    THEN("the device has a clock shows a single entry")
    {
      REQUIRE(device.clock().size() == 1);
    }

    WHEN("creating a second device")
    {
      Cashmere::Device second;
      THEN("the devices belong to different pools")
      {
        REQUIRE(device.poolId() != second.poolId());
      }
    }
  }
}
