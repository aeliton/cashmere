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

#include "journal.h"
#include "test/gtest/brokermock.h"

using namespace Cashmere;

TEST(Journal, UpdatePreemptivellyTheLocalCacheOnConnect)
{
  const auto aa = std::make_shared<Journal>(0xAA);
  const auto bb = std::make_shared<BrokerMock>();

  aa->append(10);

  aa->connect(Connection{
    BrokerStub{bb}, 1, Clock{},
    IdConnectionInfoMap{{0xBB, {.distance = 1, .version = {}}}}
  });

  const auto sources = IdConnectionInfoMap{
    {0xAA, {.distance = 0, .version = Clock{{0xAA, 1}}}},
    {0xBB, {.distance = 1, .version = Clock{{0xAA, 1}}}}
  };

  EXPECT_EQ(aa->provides(), sources);

  const auto versions = IdClockMap{{0xAA, {{0xAA, 1}}}, {0xBB, {{0xAA, 1}}}};
  EXPECT_EQ(aa->versions(), versions);
}
