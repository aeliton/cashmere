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

#include "cashmere/ledger.h"

using namespace Cashmere;

class LedgerTest : public testing::Test
{
protected:
  const ClockEntryMap rows = {
    {Clock{{0xAA, 1}}, {Clock{{0xBB, 1}}, Data{0xBB, 100, Clock{{0xAA, 1}}}}}
  };
};

using Action = Ledger::Action;
using ActionClock = Ledger::ActionClock;

TEST(Ledger, IgnoreEarlierEntryOnReplace)
{
  Entry existing = {{{0xAA, 1}}, {0xAA, 1, {}}};
  Entry incoming = {{{0xAA, 1}, {0xBB, 1}}, {0xBB, 10, {}}};
  const auto expected = ActionClock{Action::Ignore, {}};
  ASSERT_EQ(Ledger::Replaces(existing, incoming), expected);
}

TEST_F(LedgerTest, IgnoreExisting)
{
  Entry incoming = {{{0xAA, 1}}, {0xAA, 1, {}}};
  const auto expected = ActionClock{Action::Ignore, {}};
  ASSERT_EQ(Ledger::Evaluate(rows, incoming), expected);
}

TEST_F(LedgerTest, InsertNonExisting)
{
  const Entry incoming = {{{0xAA, 2}}, {0xAA, 20, {}}};
  const auto expected = ActionClock{Action::Insert, {{0xAA, 2}}};
  ASSERT_EQ(Ledger::Evaluate(rows, incoming), expected);
}

TEST_F(LedgerTest, InsertEditTypeIfNotExisting)
{
  const Entry incoming = {{{0xBB, 1}}, {0xBB, 20, {{0xAA, 2}}}};
  const auto expected = ActionClock{Action::Insert, {{0xAA, 2}}};
  ASSERT_EQ(Ledger::Evaluate(rows, incoming), expected);
}

TEST(Ledger, TwoEntries)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xFF, 2}}, Data{0xFF, 200, Clock{}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 500);
}

TEST(Ledger, SimpleEdit)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 50);
}

TEST(Ledger, SmallerIdEditsBiggerIdOnce)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
  };
  ASSERT_EQ(Ledger::Balance(entries), 50);
}

TEST(Ledger, SmallerIdEditsBiggerIdTwice)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
    {Clock{{0xAA, 2}, {0xFF, 1}}, Data{0xAA, 25, Clock{{0xFF, 1}}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 25);
}

TEST(Ledger, BiggerClockWinsOnConflictingEdits)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 100, Clock{}}},
    {Clock{{0xFF, 2}}, Data{0xFF, 200, Clock{{0xFF, 1}}}},
    {Clock{{0xAA, 1}, {0xFF, 2}}, Data{0xAA, 300, Clock{{0xFF, 1}}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 300);
}

TEST(Ledger, BiggerIdIsChosenWhenSmallerIdEditsFirst)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
    {Clock{{0xFF, 2}}, Data{0xFF, 10, Clock{{0xFF, 1}}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 10);
}

TEST(Ledger, BiggerIdIsChosenWhenBiggerIdEdditsFirst)
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xFF, 2}}, Data{0xFF, 10, Clock{{0xFF, 1}}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}}
  };
  ASSERT_EQ(Ledger::Balance(entries), 10);
}
