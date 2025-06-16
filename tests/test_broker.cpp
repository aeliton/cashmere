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
#include <catch2/catch_all.hpp>

#include "broker.h"
#include "fixtures.h"

namespace Catch
{
template<>
struct StringMaker<Cashmere::IdClockMap>
{
  static std::string convert(Cashmere::IdClockMap const& m)
  {
    std::stringstream ss;
    ss << "IdClockMap{";
    if (m.size() > 0) {
      auto it = m.cbegin();
      ss << "{" << it->first << ", " << it->second << "}";
      for (++it; it != m.cend(); it++) {
        ss << ", {" << it->first << ", " << it->second << "}";
      }
    }
    ss << "}";
    return ss.str();
  }
};

template<>
struct StringMaker<Cashmere::ClockEntry>
{
  static std::string convert(Cashmere::ClockEntry const& m)
  {
    std::stringstream ss;
    ss << "{ Clock" << m.clock << "}, Entry{" << m.entry.id << ", "
       << m.entry.value << ", " << "Clock" << m.entry.alters << " }";
    return ss.str();
  }
};

template<>
struct StringMaker<Cashmere::IdConnectionInfoMap>
{
  static std::string convert(Cashmere::IdConnectionInfoMap const& m)
  {
    std::stringstream ss;
    ss << "IdConnectionInfoMap{";
    if (m.size() > 0) {
      auto it = m.cbegin();
      ss << "{" << it->first << ", { .distance: " << it->second.distance
         << ", .version: " << it->second.version << " }}";
      for (++it; it != m.cend(); it++) {
        ss << ", {" << it->first << ", { .distance: " << it->second.distance
           << ", .version: " << it->second.version << " }}";
      }
    }
    ss << "}";
    return ss.str();
  }
};
}

using namespace Cashmere;

TEST_CASE_METHOD(Broker, "broker attach ignores nullptr")
{
  REQUIRE_FALSE(attach(nullptr));
}

TEST_CASE_METHOD(Broker, "broker without journals has an empty clock")
{
  REQUIRE(clock() == Clock{});
}

TEST_CASE_METHOD(Broker, "broker without journals has an empty versions")
{
  REQUIRE(versions() == IdClockMap{});
}

SCENARIO("Journal is attached")
{
  GIVEN("an empty broker")
  {
    auto broker0 = std::make_shared<Broker>();

    WHEN("the journal with an entry is attached")
    {
      auto aa = std::make_shared<SingleEntryMock>(0xAA, 10);
      const bool success = broker0->attach(aa);

      THEN("the attach is successful")
      {
        REQUIRE(success);
      }

      THEN("the broker has its clock updated")
      {
        REQUIRE(broker0->clock() == Clock{{0xAA, 1}});
      }

      THEN("the broker has its versions updated")
      {
        REQUIRE(broker0->versions() == IdClockMap{{0xAA, Clock{{0xAA, 1}}}});
      }

      THEN("the broker reports info about the attached journal")
      {
        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA, ConnectionInfo{.distance = 1, .version = Clock{{0xAA, 1}}}}
          }
        );
      }

      AND_WHEN("the journal is detached")
      {
        const bool success = broker0->detach(1);

        THEN("the operation succeeds")
        {
          REQUIRE(success);
        }

        THEN("the journal versions are preserved")
        {
          REQUIRE(
            broker0->versions() == IdClockMap{{0xAA, Clock{{aa->id(), 1}}}}
          );
        }

        THEN("the broker no longer report info about the journal")
        {
          REQUIRE(broker0->provides() == IdConnectionInfoMap{});
        }

        AND_WHEN("attempting to detach a non-attached journal")
        {
          const bool success = broker0->detach(aa->id());

          THEN("the operation fails")
          {
            REQUIRE_FALSE(success);
          }
        }
      }
    }
  }
}

SCENARIO_METHOD(BrokerWithEmptyMock, "journal get entries via broker")
{
  GIVEN("a broker with a journal attatched")
  {
    broker0->attach(aa);

    WHEN("an entry is inserted on the broker")
    {
      broker0->insert({Clock{{0xFF, 1}}, Entry{0xFF, 9, Clock{}}});

      THEN("insert is called on the attached journal")
      {
        REQUIRE(
          aa->_insertArgs == ClockEntryList{{{{0xFF, 1}}, {0xFF, 9, {}}}}
        );
      }

      THEN("the broker updates the version of the attached journal")
      {
        REQUIRE(
          broker0->versions() ==
          IdClockMap{{0xAA, Clock{{0xFF, 1}}}, {0xFF, Clock{{0xFF, 1}}}}
        );
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithAttachedSingleEntryMock, "a broker synchronizes journal entries"
)
{
  GIVEN("a broker with an attached journal with a transaction")
  {
    WHEN("attaching journal with transactions")
    {
      auto bb = std::make_shared<SingleEntryMock>(0xBB, 2);
      broker0->attach(bb);

      THEN("the broker has it's clock updated")
      {
        REQUIRE(broker0->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
      }

      THEN("the journal ids are listed as attached")
      {
        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA,
             ConnectionInfo{
               .distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}
             }},
            {0xBB,
             ConnectionInfo{
               .distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}
             }}
          }
        );
      }

      THEN("each journal receives the other journal's entry once")
      {
        REQUIRE(
          aa->_insertArgs ==
          ClockEntryList{{Clock{{0xBB, 1}}, Entry{0xBB, 2, {}}}}
        );
        REQUIRE(
          bb->_insertArgs ==
          ClockEntryList{{Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}}}
        );
      }
      AND_WHEN("a new entry is inserted in the broker")
      {
        broker0->insert(ClockEntry{{{0xCC, 1}}, {0xCC, 200, {}}});
        THEN("the broker clock is updated")
        {
          REQUIRE(broker0->clock() == Clock{{0xAA, 1}, {0xBB, 1}, {0xCC, 1}});
        }
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerAndTwoSingleEntryMocks,
  "versions of attached journals are updated when other attaches"
)
{
  GIVEN("a broker with a single journal attached")
  {
    broker0->attach(aa);

    REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
    REQUIRE(
      broker0->provides() ==
      IdConnectionInfoMap{{0xAA, {.distance = 1, .version = Clock{{0xAA, 1}}}}}
    );

    WHEN("attaching a second journal")
    {
      broker0->attach(bb);
      const size_t aaEntriesArgsSize = aa->_entriesArgs.size();
      const size_t bbEntriesArgsSize = bb->_entriesArgs.size();

      REQUIRE(
        broker0->versions() ==
        IdClockMap{
          {0xAA, {{0xAA, 1}, {0xBB, 1}}}, {0xBB, {{0xAA, 1}, {0xBB, 1}}}
        }
      );

      AND_WHEN("a journal disconnects")
      {
        const bool success = broker0->detach(2);
        REQUIRE(success);

        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA, {.distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}}}
          }
        );

        AND_WHEN("the attached journal inserts an entry")
        {
          const bool success =
            aa->insert(ClockEntry{{{0xAA, 2}}, {0xAA, 20, {}}});

          REQUIRE(success);

          REQUIRE(
            broker0->versions() ==
            IdClockMap{
              {0xAA, {{0xAA, 2}, {0xBB, 1}}}, {0xBB, {{0xAA, 1}, {0xBB, 1}}}
            }
          );

          AND_WHEN("the detached journal is re-attached")
          {
            broker0->attach(bb);
            THEN("the broker calls for new entries once on each journal")
            {
              REQUIRE(aa->_entriesArgs.size() == aaEntriesArgsSize + 1);
              REQUIRE(bb->_entriesArgs.size() == bbEntriesArgsSize + 1);
            }
            THEN("the broker requests entries that came after the last one seen"
            )
            {
              REQUIRE(bb->_entriesArgs.back() == Clock{{0xAA, 1}, {0xBB, 1}});
              REQUIRE(aa->_entriesArgs.back() == Clock{{0xAA, 1}, {0xBB, 1}});
            }

            THEN("the attaching journal receives the unseen entry")
            {
              REQUIRE(
                std::find(
                  bb->_insertArgs.cbegin(), bb->_insertArgs.cend(),
                  ClockEntry{{{0xAA, 2}}, {0xAA, 20, {}}}
                ) != bb->_insertArgs.cend()
              );
            }
            THEN("the versions reflect the exchange of entries")
            {
              REQUIRE(
                broker0->versions() ==
                IdClockMap{
                  {0xAA, {{0xAA, 2}, {0xBB, 1}}}, {0xBB, {{0xAA, 2}, {0xBB, 1}}}
                }
              );
            }
          }
        }
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithTwoAttachedSingleEntryAndOneEmpty,
  "only one attached journal is requested for entries"
)
{
  GIVEN("a broker with two non-empty journals attatched")
  {
    const auto initialCount = aa->_entriesArgs.size() + bb->_entriesArgs.size();

    WHEN("attaching a new journal")
    {
      broker0->attach(cc);

      THEN("only one of the attached journals is queried for entries")
      {
        const auto count = aa->_entriesArgs.size() + bb->_entriesArgs.size();
        REQUIRE(count == initialCount + 1);
      }
      AND_THEN("the attached journal has the entries retrieved once")
      {
        REQUIRE(cc->_entriesArgs.size() == 1);
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithAttachedSingleEntryMock, "broker-broker data exchange"
)
{
  GIVEN("an empty broker and a broker with non-empty journal attatched")
  {
    auto second = std::make_shared<Broker>();

    REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});

    WHEN("attaching the empty broker to other broker")
    {
      broker0->attach(second);

      THEN("the versions of the agregator broker do not change")
      {
        REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
      }

      THEN("the attached broker display the journal version")
      {
        REQUIRE(second->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
      }

      THEN("brokers display the journal distances and versions")
      {
        REQUIRE(
          second->provides() ==
          IdConnectionInfoMap{
            {0xAA, ConnectionInfo{.distance = 2, .version = Clock{{0xAA, 1}}}}
          }
        );
        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA, ConnectionInfo{.distance = 1, .version = Clock{{0xAA, 1}}}}
          }
        );
      }

      THEN("the attached broker can retrieve entries")
      {
        REQUIRE(
          second->entries() ==
          ClockEntryList{
            {Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}},
          }
        );
      }

      AND_WHEN("a journal attaches to the second broker")
      {
        auto bb = std::make_shared<JournalMock>(
          0xBB, ClockEntryMap{{{{0xBB, 1}}, {0xBB, 50, {}}}}
        );

        second->attach(bb);

        THEN("a single insert for data exchange call is made on both journals")
        {
          REQUIRE(
            aa->_insertArgs ==
            ClockEntryList{{Clock{{0xBB, 1}}, Entry{0xBB, 50, {}}}}
          );
          REQUIRE(
            bb->_insertArgs ==
            ClockEntryList{{Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}}}
          );
        }

        THEN("the clock of both brokers reflect the data exchange")
        {
          REQUIRE(broker0->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
          REQUIRE(second->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
        }

        THEN("brokers display the journal distances and versions")
        {
          REQUIRE(
            second->provides() ==
            IdConnectionInfoMap{
              {0xAA,
               ConnectionInfo{
                 .distance = 2, .version = Clock{{0xAA, 1}, {0xBB, 1}}
               }},
              {0xBB,
               ConnectionInfo{
                 .distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}
               }}
            }
          );

          REQUIRE(
            broker0->provides() ==
            IdConnectionInfoMap{
              {0xAA,
               ConnectionInfo{
                 .distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}
               }},
              {0xBB,
               ConnectionInfo{
                 .distance = 2, .version = Clock{{0xAA, 1}, {0xBB, 1}}
               }}
            }
          );
        }

        THEN("the clock of both journals reflect the data exchange")
        {
          REQUIRE(aa->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
          REQUIRE(bb->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
        }

        THEN("the previously attached journal receives the new entry")
        {
          REQUIRE(
            aa->entries() ==
            ClockEntryList{
              {Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}},
              {Clock{{0xBB, 1}}, Entry{0xBB, 50, {}}}
            }
          );
        }

        THEN("the recently attached journal receives the existing entries")
        {
          REQUIRE(
            bb->entries() ==
            ClockEntryList{
              {Clock{{0xAA, 1}}, Entry{0xAA, 1, {}}},
              {Clock{{0xBB, 1}}, Entry{0xBB, 50, {}}}
            }
          );
        }
      }
    }
  }
}

SCENARIO_METHOD(
  TwoBrokerWithASingleEntryMocksEach,
  "two connected brokers with a single entry journal each", "[provides]"
)
{
  GIVEN("two brokers with an attached journal each")
  {
    WHEN("attaching the two brokers")
    {
      broker0->attach(broker1);
      THEN("both brokers report they can provide data from the two journals")
      {
        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {170, {.distance = 1, .version = {{170, 1}, {187, 1}}}},
            {187, {.distance = 2, .version = {{170, 1}, {187, 1}}}}
          }
        );
        REQUIRE(
          broker1->provides() ==
          IdConnectionInfoMap{
            {170, {.distance = 2, .version = {{170, 1}, {187, 1}}}},
            {187, {.distance = 1, .version = {{170, 1}, {187, 1}}}}
          }
        );
      }
    }
  }
}

TEST_CASE_METHOD(
  BrokerWithTwoAttachedSingleEntryMocks, "broker provides differ per port"
)
{
  REQUIRE(
    broker0->provides(0) ==
    IdConnectionInfoMap{
      {170, {.distance = 1, .version = {{170, 1}, {187, 1}}}},
      {187, {.distance = 1, .version = {{170, 1}, {187, 1}}}}
    }
  );
  REQUIRE(
    broker0->provides(1) ==
    IdConnectionInfoMap{{187, {.distance = 1, .version = {{170, 1}, {187, 1}}}}}
  );
  REQUIRE(
    broker0->provides(2) ==
    IdConnectionInfoMap{{170, {.distance = 1, .version = {{170, 1}, {187, 1}}}}}
  );
}
