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
struct StringMaker<Cashmere::VersionMap>
{
  static std::string convert(Cashmere::VersionMap const& m)
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

SCENARIO_METHOD(SingleEntryMock, "broker attach records id and clock")
{
  GIVEN("a broker with an attached journal")
  {
    const bool success = broker.attach(mock);

    REQUIRE(success);

    THEN("the broker has the updated clock version of the journal")
    {
      REQUIRE(broker.versions() == VersionMap{{0xAA, Clock{{mock->id(), 1}}}});
    }

    AND_WHEN("the journal is detached")
    {
      const bool success = broker.detach(mock->id());

      THEN("the operation succeeds")
      {
        REQUIRE(success);
      }

      AND_THEN("the broker versions becames empty again")
      {
        REQUIRE(broker.versions().empty());
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

SCENARIO_METHOD(EmptyMock, "journal get entries via broker")
{
  GIVEN("a broker with a journal attatched")
  {
    broker.attach(mock);

    ClockChangeSignal emitter;
    emitter.connect(&broker, &Broker::onClockUpdate);

    WHEN("an entry is signaled to the broker")
    {
      emitter(Clock{{0xFF, 1}}, Entry{0xFF, 9, Clock{}});

      THEN("insert is called on the attached journal")
      {
        REQUIRE(mock->_insertArgs == ClockEntry{{{0xFF, 1}}, {0xFF, 9, {}}});
      }

      AND_WHEN("a journal detaches")
      {
        broker.detach(mock->id());

        THEN("the broker show only versions of attached journals")
        {
          REQUIRE(broker.versions() == VersionMap{{0xFF, {{0xFF, 1}}}});
        }
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

      THEN("the journal ids are listed as attached")
      {
        REQUIRE(broker.attachedIds() == std::set<Id>{0xAA, 0xBB});
      }

      THEN("::insert() is called once for each journal")
      {
        REQUIRE(aa->_insertCount == 1);
        REQUIRE(bb->_insertCount == 1);
      }

      THEN("::insert() is called with the other journal's entry")
      {
        REQUIRE(
          aa->_insertArgs == ClockEntry{Clock{{0xBB, 1}}, Entry{0xBB, 2, {}}}
        );
        REQUIRE(
          bb->_insertArgs == ClockEntry{Clock{{0xAA, 1}}, Entry{0xBB, 1, {}}}
        );
      }
    }
  }
}
