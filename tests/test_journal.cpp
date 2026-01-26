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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "brokermock.h"
#include "core/journal.h"
#include "cashmere/brokerstore.h"

using namespace Cashmere;

struct JournalTest : public ::testing::Test
{
  void SetUp() override {
    store = BrokerStore::create();
    journal = store->getOrCreate("cache://aa@localhost");
  }
  BrokerStoreBasePtr store;
  BrokerBasePtr journal;
};


TEST_F(JournalTest, ClockInitializesToEmpty)
{
  ASSERT_TRUE(journal->clock().empty());
}

TEST_F(JournalTest, ConstructJournalWithEntries)
{
  JournalPtr journal = std::make_shared<Journal>(
    "cache://aa@localhost", ClockDataMap{{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}}}
  );
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal->clock(), expectedClock);
}

TEST_F(JournalTest, NonExistingEntryQueryReturnsInvalidEntry)
{
  ASSERT_EQ(journal->entry(Clock{{0xAA, 1}}).valid(), false);
}

TEST_F(JournalTest, AllZeroedClockReturnsInvalid)
{
  ASSERT_EQ(journal->entry(Clock{{0xAA, 0}, {0xBB, 0}}).valid(), false);
}

TEST_F(JournalTest, RefuseToInsertExistingEntry)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});
  const auto result = journal.insert(Entry{clock, data});
  ASSERT_EQ(result.valid(), false);
}

TEST_F(JournalTest, UpdataClockOnAppend)
{
  journal->append(10);
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal->clock(), expectedClock);
}

TEST_F(JournalTest, DataIsRetrievedByClock)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});
  ASSERT_EQ(journal.entry(clock), data);
}

TEST_F(JournalTest, QueryIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  EXPECT_EQ(journal.entry(validClockWithZeroes), data);
}

TEST_F(JournalTest, ReplaceIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  journal.replace(500, validClockWithZeroes);

  const auto expectedClock = Clock{{0xAA, 2}};
  EXPECT_EQ(journal.clock(), expectedClock);
}

TEST_F(JournalTest, InsertIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xCC, 1}, {0x11, 0}, {0x22, 0}};

  const auto expectedClock = Clock{{0xAA, 1}, {0xCC, 1}};
  EXPECT_EQ(
    journal.insert(Entry{validClockWithZeroes, {0xCC, 206, {}}}), expectedClock
  );
}

TEST_F(JournalTest, EraseIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal("cache://aa@localhost", {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  EXPECT_TRUE(journal.erase(validClockWithZeroes));
}

TEST_F(JournalTest, QueryEntries)
{
  const ClockDataMap entries = {
    {Clock{{0xAA, 1}}, Data{0xAA, 1, {}}},
    {Clock{{0xBB, 1}}, Data{0xBB, 10, {}}},
    {Clock{{0xAA, 2}, {0xBB, 1}}, Data{0xAA, 2, Clock{{0xBB, 1}}}},
    {Clock{{0xCC, 1}}, Data{0xCC, 100, {}}},
  };
  Journal journal("cache://aa@localhost", entries);

  const EntryList expected{{Clock{{0xCC, 1}}, Data{0xCC, 100, {}}}};
  ASSERT_EQ(journal.query(Clock{{0xAA, 2}, {0xBB, 1}}), expected);
}

TEST_F(JournalTest, ReportsProvidesItsOwnData)
{
  Journal journal("cache://aa@localhost");
  const auto expected = SourcesMap{
    {0,
     IdConnectionInfoMap{{0xAA, ConnectionInfo{.distance = 0, .clock = Clock{}}}
     }}
  };
  ASSERT_EQ(journal.sources(), expected);
}

TEST_F(JournalTest, RefusesEntriesOutOfOrder)
{
  Journal journal("cache://aa@localhost");
  const auto clock = journal.insert(Entry{{{0xBB, 2}}, {0xBB, 10, {}}});
  ASSERT_EQ(clock.valid(), false);
}

TEST_F(JournalTest, UpdatePreemptivellyTheLocalCacheOnConnect)
{
  const auto bb = std::make_shared<BrokerMock>();

  journal->append(10);

  journal->connect(Connection(
    bb, 1, Clock{}, IdConnectionInfoMap{{0xBB, {.distance = 1, .clock = {}}}}
  ));

  const auto sources = SourcesMap{
    {0, IdConnectionInfoMap{{0xAA, {.distance = 0, .clock = Clock{{0xAA, 1}}}}}
    },
    {1, IdConnectionInfoMap{{0xBB, {.distance = 1, .clock = Clock{{0xAA, 1}}}}}}
  };

  EXPECT_EQ(journal->sources(), sources);

  const auto versions = IdClockMap{{0xAA, {{0xAA, 1}}}, {0xBB, {{0xAA, 1}}}};
  EXPECT_EQ(journal->versions(), versions);
}

TEST_F(JournalTest, RelayWithZeroedIdIsAppliedLocally)
{
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal->relay(Data{0, 999, {}}, 0), expectedClock);
}
