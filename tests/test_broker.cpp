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
    if (m.empty()) {
      return "{ }";
    }
    std::stringstream ss;
    ss << "{";
    auto it = m.cbegin();
    ss << "{" << it->first << ", " << it->second << "}";
    for (++it; it != m.cend(); it++) {
      ss << ", {" << it->first << ", " << it->second << "}";
    }
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

SCENARIO_METHOD(SingleEntryMock, "Journal is attaches to a Broker")
{
  GIVEN("a broker")
  {
    REQUIRE(broker.versions() == IdClockMap{});

    REQUIRE(mock->clockChanged().count() == 0);

    WHEN("a journal is attached to the broker")
    {
      const bool success = broker.attach(mock);
      REQUIRE(success);

      THEN("the broker has its clock updated")
      {
        REQUIRE(broker.versions() == IdClockMap{{0xAA, mock->clock()}});
      }

      THEN("the journal has a listener on it's signal object")
      {
        REQUIRE(mock->clockChanged().count() == 1);
      }

      AND_WHEN("the journal is detached")
      {
        const bool success = broker.detach(mock->id());

        THEN("the operation succeeds")
        {
          REQUIRE(success);
        }

        THEN("the journal versions are preserved")
        {
          REQUIRE(
            broker.versions() == IdClockMap{{0xAA, Clock{{mock->id(), 1}}}}
          );
        }

        THEN("the broker is removed from clock updates of the journal")
        {
          REQUIRE(mock->clockChanged().count() == 0);
        }

        AND_WHEN("attempting to detach a non-attached journal")
        {
          const bool success = broker.detach(mock->id());

          THEN("the operation fails")
          {
            REQUIRE_FALSE(success);
          }
        }
      }
    }
  }
}

SCENARIO_METHOD(EmptyMock, "journal get entries via broker")
{
  GIVEN("a broker with a journal attatched")
  {
    broker.attach(mock);

    ClockChangeSignal emitter;
    emitter.connect(&broker, &Broker::insert);

    WHEN("an entry is signaled to the broker")
    {
      emitter({Clock{{0xFF, 1}}, Entry{0xFF, 9, Clock{}}});

      THEN("insert is called on the attached journal")
      {
        REQUIRE(
          mock->_insertArgs == ClockEntryList{{{{0xFF, 1}}, {0xFF, 9, {}}}}
        );
      }
    }
  }
}

SCENARIO_METHOD(TwoSingleEntryMocks, "a broker synchronizes journal entries")
{
  GIVEN("a empty broker")
  {
    REQUIRE(broker.attachedIds() == std::set<Id>{});

    WHEN("attaching journal with transactions")
    {
      broker.attach(aa);
      broker.attach(bb);

      THEN("the broker has it's clock updated")
      {
        REQUIRE(broker.clock() == Clock{{0xAA, 1}, {0xBB, 1}});
      }

      THEN("the journal ids are listed as attached")
      {
        REQUIRE(broker.attachedIds() == std::set<Id>{0xAA, 0xBB});
      }

      THEN("::insert() is called once for each journal")
      {
        REQUIRE(aa->_insertArgs.size() == 1);
        REQUIRE(bb->_insertArgs.size() == 1);
      }

      THEN("::insert() is called with the other journal's entry")
      {
        REQUIRE(
          aa->_insertArgs ==
          ClockEntryList{{Clock{{0xBB, 1}}, Entry{0xBB, 2, {}}}}
        );
        REQUIRE(
          bb->_insertArgs ==
          ClockEntryList{{Clock{{0xAA, 1}}, Entry{0xBB, 1, {}}}}
        );
      }
      AND_WHEN("a new entry is inserted in the broker")
      {
        broker.insert(ClockEntry{{{0xCC, 1}}, {0xCC, 200, {}}});
        THEN("the broker clock is updated")
        {
          REQUIRE(broker.clock() == Clock{{0xAA, 1}, {0xBB, 1}, {0xCC, 1}});
        }
      }
    }
  }
}

SCENARIO_METHOD(TwoSingleEntryMocks, "Broker sends only new transactions")
{
  GIVEN("a broker with two journals attached")
  {
    broker.attach(aa);
    broker.attach(bb);

    REQUIRE(broker.attachedIds().size() == 2);

    const size_t aaEntriesArgsSize = aa->_entriesArgs.size();
    const size_t bbEntriesArgsSize = bb->_entriesArgs.size();

    WHEN("a journal disconnects")
    {
      broker.detach(bb->id());

      REQUIRE(broker.attachedIds() == std::set<Id>{aa->id()});

      AND_WHEN("the attached journal inserts an entry")
      {
        aa->insert(ClockEntry{{{aa->id(), 2}}, {aa->id(), 20, {}}});

        AND_WHEN("the detached journal is re-attached")
        {
          broker.attach(bb);
          THEN("the broker calls for new entries once on each journal")
          {
            REQUIRE(aa->_entriesArgs.size() == aaEntriesArgsSize + 1);
            REQUIRE(bb->_entriesArgs.size() == bbEntriesArgsSize + 1);
          }
          THEN("the broker requests entries that came after the last one seen")
          {
            REQUIRE(bb->_entriesArgs.back() == Clock{{bb->id(), 1}});
            REQUIRE(aa->_entriesArgs.back() == Clock{{bb->id(), 1}});
          }
        }
      }
    }
  }
}

SCENARIO_METHOD(
  TwoAttachedSingleEntryOneEmpty,
  "only one attached journal is requested for entries"
)
{
  GIVEN("a broker with two non-empty journals attatched")
  {
    const auto initialCount = aa->_entriesArgs.size() + bb->_entriesArgs.size();

    WHEN("attaching a new journal")
    {
      broker.attach(cc);

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
