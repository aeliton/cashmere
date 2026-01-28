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

#include "cashmere/entry.h"

using namespace Cashmere;

TEST(Data, ParseEmpty)
{
  std::istringstream ins("{0, 0, {}}");
  Data data;
  EXPECT_TRUE(Data::Read(ins, data));
  ASSERT_EQ(data, Data{});
}

TEST(Data, ParseDataWithEmptyClock)
{
  std::istringstream ins("{aa, 10, {}}");
  Data data;
  EXPECT_TRUE(Data::Read(ins, data));

  const auto expectedData = Data{0xAA, 10, {}};
  ASSERT_EQ(data, expectedData);
}

TEST(Data, HexadeciamValueIsNotValid)
{
  std::istringstream ins("{aa, aa, {}}");
  Data data;
  ASSERT_EQ(Data::Read(ins, data), false);
}

TEST(Data, IdMustBeAValidHexadecimalNunber)
{
  std::istringstream ins("{zz, 1, {}}");
  Data data;
  ASSERT_EQ(Data::Read(ins, data), false);
}

TEST(Entry, ValidEmptyEntry)
{
  std::istringstream ins("{{}, {0, 0, {}}}");
  Entry data;
  EXPECT_TRUE(Entry::Read(ins, data));
  ASSERT_EQ(data, Entry{});
}

TEST(Entry, ValidNonEmptyEntry)
{
  std::istringstream ins("{{{aa, 1}}, {aa, 10, {}}}");
  Entry data;
  EXPECT_TRUE(Entry::Read(ins, data));
  const auto expectedEntry = Entry{{{0xAA, 1}}, {0xAA, 10, {}}};
  ASSERT_EQ(data, expectedEntry);
}
