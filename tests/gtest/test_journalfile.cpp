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

#include <filesystem>
#include <fstream>

#include "test/gtest/brokermock.h"
#include "test/gtest/fixtures.h"

using namespace Cashmere;
using ::testing::Return;

TEST_F(JournalFileTest, SeparateFilesPerJournal)
{
  const std::string bbFilename = fs::path(tmpdir) / "00000000000000bb";

  const auto broker = std::make_shared<BrokerMock>();

  EXPECT_CALL(
    *broker,
    connect(Connection{
      BrokerStub(journal), 1, {}, {{kFixtureId, {.distance = 1, .version = {}}}}
    })
  )
    .Times(1)
    .WillOnce(Return(Connection{
      BrokerStub{}, 1, Clock{{0xBB, 1}},
      IdConnectionInfoMap{{0xBB, {1, Clock{{0xBB, 1}}}}}
    }));
  EXPECT_CALL(*broker, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}}}));

  journal->connect(BrokerStub{broker});

  std::ifstream file(journal->filename());
  EXPECT_EQ(file.peek(), std::ifstream::traits_type::eof());

  EXPECT_TRUE(fs::exists(bbFilename));
  std::string line;
  getline(std::ifstream(bbFilename), line);
  EXPECT_EQ(line, "{{{bb, 1}}, {bb, 10, {}}}");
}
