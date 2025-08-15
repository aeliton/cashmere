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
#include <gtest/gtest.h>

#include "cashmere/clock.h"

using namespace Cashmere;

TEST(Clock, ConcurrentClocks)
{
  const auto clock = Clock{{0xAA, 2}, {0xBB, 2}};
  ASSERT_TRUE(clock.concurrent(Clock{{0xCC, 1}}));
}

TEST(Clock, ClockNotSmallerThanItself)
{
  const auto clock = Clock{{0xAA, 1}};
  ASSERT_EQ(clock.smallerThan(clock), false);
}

TEST(Clock, ClockWithExtraEntryIsBigger)
{
  const auto smaller = Clock{{0xFF, 2}};
  const auto bigger = Clock{{0xAA, 1}, {0xFF, 2}};

  ASSERT_TRUE(smaller.smallerThan(bigger));
}

TEST(Clock, ClockDoesNotConcurrsWithItself)
{
  const auto clock = Clock{{0xAA, 1}};
  ASSERT_EQ(clock.concurrent(clock), false);
}

TEST(Clock, ParseInvalidStrings) {}

class StringTest : public ::testing::TestWithParam<std::string>
{
};

TEST_P(StringTest, FromInvalidString)
{
  std::istringstream ins(GetParam());
  Clock clock;
  ASSERT_EQ(Clock::Read(ins, clock), false);
}

class StringToClockTest
  : public ::testing::TestWithParam<std::tuple<std::string, Clock>>
{
};

TEST_P(StringToClockTest, FromString)
{
  const auto [input, expected] = GetParam();
  std::istringstream ins(input);
  Clock clock;
  EXPECT_TRUE(Clock::Read(ins, clock));
  ASSERT_EQ(clock, expected);
}

INSTANTIATE_TEST_SUITE_P(
  Clock, StringTest,
  ::testing::Values("{{zz, 1}}", "{{ AA, aa}}", "", "{{AA, 1}", "{AA, 1}")
);

INSTANTIATE_TEST_SUITE_P(
  Clock, StringToClockTest,
  ::testing::Values(
    std::tuple<std::string, Clock>{"{}", Clock{}},
    std::tuple<std::string, Clock>{" {}", Clock{}},
    std::tuple<std::string, Clock>{"{ }", Clock{}},
    std::tuple<std::string, Clock>{"{{AA, 1}}", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{"{{ AA, 1}}", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{"{{AA , 1}}", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{"{{AA,  1}}", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{"{{AA, 1 }}", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{"{{AA, 1} }", Clock{{0xAA, 1}}},
    std::tuple<std::string, Clock>{
      " { { AA ,  1} ,  { BB , 1 } }", Clock{{0xAA, 1}, {0xBB, 1}}
    }
  )
);
