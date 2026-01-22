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
#include "core.h"
#include "cashmere/journal.h"

using namespace Cashmere;

TEST(Journal, ClockInitializesToEmpty)
{
  JournalPtr journal = std::make_shared<Journal>();
  ASSERT_TRUE(journal->clock().empty());
}

TEST(Journal, ConstructJournalWithEntries)
{
  JournalPtr journal = std::make_shared<Journal>(
    0xAA, ClockDataMap{{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}}}
  );
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal->clock(), expectedClock);
}

TEST(Journal, NonExistingEntryQueryReturnsInvalidEntry)
{
  Journal journal;
  ASSERT_EQ(journal.entry(Clock{{0xAA, 1}}).valid(), false);
}

TEST(Journal, AllZeroedClockReturnsInvalid)
{
  Journal journal;
  ASSERT_EQ(journal.entry(Clock{{0xAA, 0}, {0xBB, 0}}).valid(), false);
}

TEST(Journal, RefuseToInsertExistingEntry)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});
  const auto result = journal.insert(Entry{clock, data});
  ASSERT_EQ(result.valid(), false);
}

TEST(Journal, UpdataClockOnAppend)
{
  Journal journal(0xAA);
  journal.append(10);
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal.clock(), expectedClock);
}

TEST(Journal, DataIsRetrievedByClock)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});
  ASSERT_EQ(journal.entry(clock), data);
}

TEST(Journal, QueryIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  EXPECT_EQ(journal.entry(validClockWithZeroes), data);
}

TEST(Journal, ReplaceIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  journal.replace(500, validClockWithZeroes);

  const auto expectedClock = Clock{{0xAA, 2}};
  EXPECT_EQ(journal.clock(), expectedClock);
}

TEST(Journal, InsertIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xCC, 1}, {0x11, 0}, {0x22, 0}};

  const auto expectedClock = Clock{{0xAA, 1}, {0xCC, 1}};
  EXPECT_EQ(
    journal.insert(Entry{validClockWithZeroes, {0xCC, 206, {}}}), expectedClock
  );
}

TEST(Journal, EraseIgnoreZeroedClockEntries)
{
  const auto clock = Clock{{0xAA, 1}};
  const auto data = Data{0xAA, 10, {}};
  Journal journal(0xAA, {{clock, data}});

  const auto validClockWithZeroes = Clock{{0xAA, 1}, {0x11, 0}, {0x22, 0}};

  EXPECT_TRUE(journal.erase(validClockWithZeroes));
}

TEST(Journal, QueryEntries)
{
  const ClockDataMap entries = {
    {Clock{{0xAA, 1}}, Data{0xAA, 1, {}}},
    {Clock{{0xBB, 1}}, Data{0xBB, 10, {}}},
    {Clock{{0xAA, 2}, {0xBB, 1}}, Data{0xAA, 2, Clock{{0xBB, 1}}}},
    {Clock{{0xCC, 1}}, Data{0xCC, 100, {}}},
  };
  Journal journal(0xAA, entries);

  const EntryList expected{{Clock{{0xCC, 1}}, Data{0xCC, 100, {}}}};
  ASSERT_EQ(journal.query(Clock{{0xAA, 2}, {0xBB, 1}}), expected);
}

TEST(Journal, ReportsProvidesItsOwnData)
{
  Journal journal(0xAA);
  const auto expected = SourcesMap{
    {0,
     IdConnectionInfoMap{{0xAA, ConnectionInfo{.distance = 0, .clock = Clock{}}}
     }}
  };
  ASSERT_EQ(journal.sources(), expected);
}

TEST(Journal, RefusesEntriesOutOfOrder)
{
  Journal journal(0xAA);
  const auto clock = journal.insert(Entry{{{0xBB, 2}}, {0xBB, 10, {}}});
  ASSERT_EQ(clock.valid(), false);
}

TEST(Journal, UpdatePreemptivellyTheLocalCacheOnConnect)
{
  const auto aa = BrokerStore::instance()->build("cache://aa@localhost");
  EXPECT_EQ(aa->id(), 0xAA);
  
  const auto bb = std::make_shared<BrokerMock>();

  aa->append(10);

  aa->connect(Connection(
    bb, 1, Clock{}, IdConnectionInfoMap{{0xBB, {.distance = 1, .clock = {}}}}
  ));

  const auto sources = SourcesMap{
    {0, IdConnectionInfoMap{{0xAA, {.distance = 0, .clock = Clock{{0xAA, 1}}}}}
    },
    {1, IdConnectionInfoMap{{0xBB, {.distance = 1, .clock = Clock{{0xAA, 1}}}}}}
  };

  EXPECT_EQ(aa->sources(), sources);

  const auto versions = IdClockMap{{0xAA, {{0xAA, 1}}}, {0xBB, {{0xAA, 1}}}};
  EXPECT_EQ(aa->versions(), versions);
}

TEST(Journal, RelayWithZeroedIdIsAppliedLocally)
{
  const auto journal = BrokerStore::instance()->build("cache://aa@localhost");
  const auto expectedClock = Clock{{0xAA, 1}};
  ASSERT_EQ(journal->relay(Data{0, 999, {}}, 0), expectedClock);
}
