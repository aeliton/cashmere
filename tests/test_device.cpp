#include "device.h"
#include <catch2/catch_all.hpp>

SCENARIO("device adds and edits transactions")
{
  GIVEN("an empty device object")
  {
    Cashmere::Device device;
    THEN("the device vector clock shows a single entry")
    {
      REQUIRE(device.clock().size() == 1);
    }
    AND_THEN("the device id is zero")
    {
      REQUIRE(device.id() == 0);
    }
  }
}
