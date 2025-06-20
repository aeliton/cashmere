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
#include "signal/signal.h"
#include <catch2/catch_all.hpp>

using namespace Signaller;

SCENARIO("signal to a single slot")
{
  GIVEN("a signal object with a lambda connected to it")
  {
    Signal<void(int)> signal;

    int calledWith = 0xFFFFFFFF;
    Signal<void(int)>::Slot lambda = [&calledWith](int a) { calledWith = a; };
    const Connection conn = signal.connect(lambda);

    REQUIRE(signal.count() == 1);

    WHEN("trigerring the signal")
    {
      signal(10);
      THEN("the lambda has ben executed with the passed value")
      {
        REQUIRE(calledWith == 10);
      }
      AND_WHEN("disconnecting the lambda")
      {
        const bool success = signal.disconnect(conn);
        THEN("the disconnection succeeds")
        {
          REQUIRE(success);
        }
        AND_THEN("the connection count decreases")
        {
          REQUIRE(signal.count() == 0);
        }

        AND_WHEN("re-triggering the signal")
        {
          signal(11);
          THEN("the lamda is not called")
          {
            REQUIRE(calledWith == 10);
          }
        }
        AND_WHEN("calling disconnect again with the same connection")
        {
          THEN("it fails")
          {
            REQUIRE_FALSE(signal.disconnect(conn));
          }
        }
      }
    }

    WHEN("passing a member method as a slot")
    {
      struct A
      {
        void slot(int x)
        {
          value = x;
        }
        int value = 0;
      };

      A a;

      REQUIRE(a.value == 0);

      const Connection conn = signal.connect(&a, &A::slot);

      AND_WHEN("triggering the signal")
      {
        signal(20);
        THEN("the member method is called")
        {
          REQUIRE(a.value == 20);
        }
        AND_THEN("the lambda is also called with the same value")
        {
          REQUIRE(calledWith == 20);
        }

        AND_WHEN("disconnecting the member slot")
        {
          const bool success = signal.disconnect(conn);
          THEN("it succeeds")
          {
            REQUIRE(success);
          }
          AND_WHEN("re-triggering the signal")
          {
            signal(33);
            THEN("the callback is not called")
            {
              REQUIRE(a.value == 20);
            }
          }
        }
      }
    }
  }
}

SCENARIO("using a member with multiple arguments")
{
  GIVEN("a signal object with a lambda connected to it")
  {
    struct A
    {
      void slot(int a, int b)
      {
        value = a + b;
      }
      int value = 0;
    };
    Signal<void(int, int)> signal;

    WHEN("passing a member method as a slot")
    {
      A a;

      REQUIRE(a.value == 0);

      signal.connect(&a, &A::slot);

      AND_WHEN("triggering the signal")
      {
        signal(20, 30);
        THEN("the member method is called")
        {
          REQUIRE(a.value == 50);
        }
      }
    }
  }
}
