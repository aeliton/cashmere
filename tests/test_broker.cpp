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
struct StringMaker<Cashmere::Entry>
{
  static std::string convert(Cashmere::Entry const& m)
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

TEST_CASE_METHOD(Broker, "broker connect ignores nullptr")
{
  REQUIRE(connect(nullptr) == -1);
}

TEST_CASE_METHOD(Broker, "broker without journals has an empty clock")
{
  REQUIRE(clock() == Clock{});
}

TEST_CASE_METHOD(Broker, "broker without journals has an empty versions")
{
  REQUIRE(versions() == IdClockMap{});
}

SCENARIO("Journal is connected")
{
  GIVEN("an empty broker")
  {
    auto broker0 = std::make_shared<Broker>();

    WHEN("the journal with an entry is connected")
    {
      auto aa = std::make_shared<SingleEntryMock>(0xAA, 10);
      const bool success = broker0->connect(aa);

      THEN("the connect is successful")
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

      THEN("the broker reports info about the connected journal")
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
        const bool success = broker0->disconnect(1);

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

        AND_WHEN("attempting to detach a non-connected journal")
        {
          const Port port = broker0->disconnect(aa->id());

          THEN("the operation fails")
          {
            REQUIRE(port < 0);
          }
        }
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithAttachedSingleEntryMock, "a broker synchronizes journal entries"
)
{
  GIVEN("a broker with a connected journal with a transaction")
  {
    WHEN("connecting journal with transactions")
    {
      auto bb = std::make_shared<SingleEntryMock>(0xBB, 2);
      broker0->connect(bb);

      THEN("the broker has it's clock updated")
      {
        REQUIRE(broker0->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
      }

      THEN("the journal ids are listed as connected")
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
          aa->_insertArgs == EntryList{{Clock{{0xBB, 1}}, Data{0xBB, 2, {}}}}
        );
        REQUIRE(
          bb->_insertArgs == EntryList{{Clock{{0xAA, 1}}, Data{0xAA, 1, {}}}}
        );
      }

      AND_WHEN("a new entry is inserted in the broker")
      {
        broker0->insert(Entry{{{0xCC, 1}}, {0xCC, 200, {}}});

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
  "versions of connected journals are updated when other connectes"
)
{
  GIVEN("a broker with a single journal connected")
  {
    broker0->connect(aa);

    REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
    REQUIRE(
      broker0->provides() ==
      IdConnectionInfoMap{{0xAA, {.distance = 1, .version = Clock{{0xAA, 1}}}}}
    );

    WHEN("connecting a second journal")
    {
      broker0->connect(bb);
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
        const bool success = broker0->disconnect(2);
        REQUIRE(success);

        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA, {.distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}}}
          }
        );

        AND_WHEN("the connected journal inserts an entry")
        {
          const auto result = aa->insert(Entry{{{0xAA, 2}}, {0xAA, 20, {}}});

          REQUIRE(result == Clock{{0xAA, 2}, {0xBB, 1}});

          REQUIRE(
            broker0->versions() ==
            IdClockMap{
              {0xAA, {{0xAA, 2}, {0xBB, 1}}}, {0xBB, {{0xAA, 1}, {0xBB, 1}}}
            }
          );

          AND_WHEN("the detached journal is re-connected")
          {
            broker0->connect(bb);
            THEN("the broker calls for new entries once on each journal")
            {
              REQUIRE(aa->_entriesArgs.size() == aaEntriesArgsSize + 1);
              REQUIRE(bb->_entriesArgs.size() == bbEntriesArgsSize + 1);
            }
            THEN("the broker requests entries that came after the last one seen"
            )
            {
              REQUIRE(bb->_entriesArgs.back() == Clock{{0xAA, 2}, {0xBB, 1}});
              REQUIRE(aa->_entriesArgs.back() == Clock{{0xAA, 1}, {0xBB, 1}});
            }

            THEN("the connecting journal receives the unseen entry")
            {
              REQUIRE(
                std::find(
                  bb->_insertArgs.cbegin(), bb->_insertArgs.cend(),
                  Entry{{{0xAA, 2}}, {0xAA, 20, {}}}
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
  "only one connected journal is requested for entries"
)
{
  GIVEN("a broker with two non-empty journals attatched")
  {
    const auto initialCount = aa->_entriesArgs.size() + bb->_entriesArgs.size();

    WHEN("connecting a new journal")
    {
      broker0->connect(cc);

      THEN("only one of the connected journals is queried for entries")
      {
        const auto count = aa->_entriesArgs.size() + bb->_entriesArgs.size();
        REQUIRE(count == initialCount + 1);
      }
      AND_THEN("the connected journal has the entries retrieved once")
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
    auto broker1 = std::make_shared<Broker>();

    REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});

    WHEN("connecting the empty broker to other broker")
    {
      broker0->connect(broker1);

      THEN("the versions of the agregator broker do not change")
      {
        REQUIRE(broker0->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
      }

      THEN("the connected broker display the journal version")
      {
        REQUIRE(broker1->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
      }

      THEN("brokers display the journal distances and versions")
      {
        REQUIRE(
          broker1->provides() ==
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

      THEN("the connected broker can retrieve entries")
      {
        REQUIRE(
          broker1->entries() ==
          EntryList{
            {Clock{{0xAA, 1}}, Data{0xAA, 1, {}}},
          }
        );
      }

      AND_WHEN("a journal connectes to the second broker")
      {
        auto bb = std::make_shared<JournalMock>(
          0xBB, ClockDataMap{{{{0xBB, 1}}, {0xBB, 50, {}}}}
        );

        broker1->connect(bb);

        THEN("a single insert for data exchange call is made on both journals")
        {
          REQUIRE(
            aa->_insertArgs == EntryList{{Clock{{0xBB, 1}}, Data{0xBB, 50, {}}}}
          );
          REQUIRE(
            bb->_insertArgs == EntryList{{Clock{{0xAA, 1}}, Data{0xAA, 1, {}}}}
          );
        }

        THEN("the clock of both brokers reflect the data exchange")
        {
          REQUIRE(broker0->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
          REQUIRE(broker1->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
        }

        THEN("brokers display the journal distances and versions")
        {
          REQUIRE(
            broker1->provides() ==
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

        THEN("the previously connected journal receives the new entry")
        {
          REQUIRE(
            aa->entries() ==
            EntryList{
              {Clock{{0xAA, 1}}, Data{0xAA, 1, {}}},
              {Clock{{0xBB, 1}}, Data{0xBB, 50, {}}}
            }
          );
        }

        THEN("the recently connected journal receives the existing entries")
        {
          REQUIRE(
            bb->entries() ==
            EntryList{
              {Clock{{0xAA, 1}}, Data{0xAA, 1, {}}},
              {Clock{{0xBB, 1}}, Data{0xBB, 50, {}}}
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
  GIVEN("two brokers with a connected journal each")
  {
    WHEN("connecting the two brokers")
    {
      broker0->connect(broker1);
      THEN("both brokers report they can provide data from the two journals")
      {
        REQUIRE(
          broker0->provides() ==
          IdConnectionInfoMap{
            {0xAA, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}},
            {0xBB, {.distance = 2, .version = {{0xAA, 1}, {0xBB, 1}}}}
          }
        );
        REQUIRE(
          broker1->provides() ==
          IdConnectionInfoMap{
            {0xAA, {.distance = 2, .version = {{0xAA, 1}, {0xBB, 1}}}},
            {0xBB, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}}
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
      {0xAA, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}},
      {0xBB, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}}
    }
  );
  REQUIRE(
    broker0->provides(1) ==
    IdConnectionInfoMap{
      {0xBB, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}}
    }
  );
  REQUIRE(
    broker0->provides(2) ==
    IdConnectionInfoMap{
      {0xAA, {.distance = 1, .version = {{0xAA, 1}, {0xBB, 1}}}}
    }
  );
}
