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
    ss << "{ Clock" << m.clock << "}, Entry{" << m.entry.journalId << ", "
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
         << ", .version: " << it->second.version << "} }";
      for (++it; it != m.cend(); it++) {
        ss << ", {" << it->first << ", { .distance: " << it->second.distance
           << ", .version: " << it->second.version << "} }";
      }
    }
    ss << "}";
    return ss.str();
  }
};
}

using namespace Cashmere;

TEST_CASE("broker attach ignores nullptr")
{
  Broker broker;
  REQUIRE_FALSE(broker.attach(nullptr));
}

TEST_CASE("broker without journals has an empty clock")
{
  Broker broker;
  REQUIRE(broker.clock() == Clock{});
}

SCENARIO_METHOD(BrokerWithSingleEntryMock, "Journal is attached to a Broker")
{
  GIVEN("a broker")
  {
    REQUIRE(broker->versions() == IdClockMap{});

    REQUIRE(mock->clock() == Clock{{0xAA, 1}});

    WHEN("a journal is attached to the broker")
    {
      const bool success = broker->attach(mock);
      REQUIRE(success);

      THEN("the broker has its clock updated")
      {
        REQUIRE(broker->versions() == IdClockMap{{0xAA, Clock{{0xAA, 1}}}});
      }

      AND_WHEN("the journal is detached")
      {
        const bool success = broker->detach(1);

        THEN("the operation succeeds")
        {
          REQUIRE(success);
        }

        THEN("the journal versions are preserved")
        {
          REQUIRE(
            broker->versions() == IdClockMap{{0xAA, Clock{{mock->id(), 1}}}}
          );
        }

        AND_WHEN("attempting to detach a non-attached journal")
        {
          const bool success = broker->detach(mock->id());

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
    broker->attach(mock);

    WHEN("an entry is inserted on the broker")
    {
      broker->insert({Clock{{0xFF, 1}}, Entry{0xFF, 9, Clock{}}});

      THEN("insert is called on the attached journal")
      {
        REQUIRE(
          mock->_insertArgs == ClockEntryList{{{{0xFF, 1}}, {0xFF, 9, {}}}}
        );
      }

      THEN("the broker updates the version of the attached journal")
      {
        REQUIRE(
          broker->versions() ==
          IdClockMap{{0xAA, Clock{{0xFF, 1}}}, {0xFF, Clock{{0xFF, 1}}}}
        );
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithTwoSingleEntryMocks, "a broker synchronizes journal entries"
)
{
  GIVEN("a empty broker")
  {
    REQUIRE(broker->provides() == IdConnectionInfoMap{});

    WHEN("attaching journal with transactions")
    {
      broker->attach(aa);
      broker->attach(bb);

      THEN("the broker has it's clock updated")
      {
        REQUIRE(broker->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
      }

      THEN("the journal ids are listed as attached")
      {
        REQUIRE(
          broker->provides() ==
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
        broker->insert(ClockEntry{{{0xCC, 1}}, {0xCC, 200, {}}});
        THEN("the broker clock is updated")
        {
          REQUIRE(broker->clock() == Clock{{0xAA, 1}, {0xBB, 1}, {0xCC, 1}});
        }
      }
    }
  }
}

SCENARIO_METHOD(
  BrokerWithTwoSingleEntryMocks,
  "versions of attached journals are updated when other attaches"
)
{
  GIVEN("a broker with a single journal attached")
  {
    broker->attach(aa);

    REQUIRE(broker->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
    REQUIRE(
      broker->provides() ==
      IdConnectionInfoMap{{0xAA, {.distance = 1, .version = Clock{{0xAA, 1}}}}}
    );

    WHEN("attaching a second journal")
    {
      broker->attach(bb);
      const size_t aaEntriesArgsSize = aa->_entriesArgs.size();
      const size_t bbEntriesArgsSize = bb->_entriesArgs.size();

      REQUIRE(
        broker->versions() ==
        IdClockMap{
          {0xAA, {{0xAA, 1}, {0xBB, 1}}}, {0xBB, {{0xAA, 1}, {0xBB, 1}}}
        }
      );

      AND_WHEN("a journal disconnects")
      {
        const bool success = broker->detach(2);
        REQUIRE(success);

        REQUIRE(
          broker->provides() ==
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
            broker->versions() ==
            IdClockMap{
              {0xAA, {{0xAA, 2}, {0xBB, 1}}}, {0xBB, {{0xAA, 1}, {0xBB, 1}}}
            }
          );

          AND_WHEN("the detached journal is re-attached")
          {
            broker->attach(bb);
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
                broker->versions() ==
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
      broker->attach(cc);

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

    REQUIRE(broker->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});

    WHEN("attaching the empty broker to other broker")
    {
      broker->attach(second);

      THEN("the versions of the agregator broker do not change")
      {
        REQUIRE(broker->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
      }

      THEN("the attached broker display the journal version")
      {
        REQUIRE(second->versions() == IdClockMap{{0xAA, {{0xAA, 1}}}});
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
          REQUIRE(mock->_insertArgs.size() == 1);
          REQUIRE(bb->_insertArgs.size() == 1);
        }

        THEN("the clock of both journals reflect the data exchange")
        {
          REQUIRE(mock->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
          REQUIRE(bb->clock() == Clock{{0xAA, 1}, {0xBB, 1}});
        }

        THEN("the previously attached journal receives the new entry")
        {
          REQUIRE(
            mock->entries() ==
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
