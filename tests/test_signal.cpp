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
#include "signal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

SCENARIO("signal to a single slot")
{
  GIVEN("a signal object with a lambda connected to it")
  {
    Signal<void, int> signal;

    int calledWith = 0xFFFFFFFF;
    Signal<void, int>::Slot lambda = [&calledWith](int a) { calledWith = a; };
    signal.connect(lambda);

    WHEN("trigerring the signal")
    {
      signal(10);
      THEN("the lambda has ben executed with the passed value")
      {
        REQUIRE(calledWith == 10);
      }
    }
  }
}
