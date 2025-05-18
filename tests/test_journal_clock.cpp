#include "journal.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

using Clock = Journal::Clock;
using Operation = Journal::Operation;

SCENARIO("journal clock updates when entries are changed")
{
  GIVEN("an empty journal object")
  {
    Journal journal;
    THEN("the journal vector clock is empty")
    {
      REQUIRE(journal.clock() == Clock{});
    }
    WHEN("adding a entry with the journal ID")
    {
      journal.append(1000);
      const auto kId = journal.id();
      THEN("the clock is updated")
      {
        REQUIRE(journal.clock() == Clock{{kId, 1}});
      }

      AND_WHEN("adding an entry with an different journal ID")
      {
        journal.append(0xAA, 200);
        THEN("the clock is updated")
        {
          REQUIRE(journal.clock() == Clock{{kId, 1}, {0xAA, 1}});
        }
        AND_WHEN("another journal replaces a transaction")
        {
          journal.replace(0xFF, 300, {{kId, 1}});
          THEN("the actor's id is incremented")
          {
            REQUIRE(journal.clock() == Clock{{kId, 1}, {0xAA, 1}, {0xFF, 1}});
          }
          AND_WHEN("an entry is deleted")
          {
            journal.erase({{kId, 1}});
            THEN("the actor's is incremented")
            {
              REQUIRE(journal.clock() == Clock{{kId, 2}, {0xAA, 1}, {0xFF, 1}});
            }
          }
        }
      }
    }
  }
}
