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

#include "brokermock.h"
#include "journalfile.h"

using namespace Cashmere;
using ::testing::Return;

namespace fs = std::filesystem;

constexpr Id kFixtureId = 0xbaadcafe;
constexpr char const* kFixtureIdStr = "00000000baadcafe";

struct JournalFileTest : public ::testing::Test
{
  void SetUp() override
  {
    tmpdir = fs::temp_directory_path() / std::to_string(Random{}.next());
    filename = fs::path(tmpdir) / kFixtureIdStr;
    assert(!fs::exists(tmpdir));
    journal = std::make_shared<JournalFile>(kFixtureId, tmpdir);
  }
  void TearDown() override
  {
    const fs::path directory = fs::path(tmpdir);
    assert(fs::exists(directory));
    [[maybe_unused]] const bool deleted = fs::remove_all(directory);
    assert(deleted);
  }
  std::string tmpdir;
  std::string filename;
  JournalFilePtr journal;
};

TEST_F(JournalFileTest, SeparateFilesPerJournal)
{
  const std::string bbFilename = fs::path(tmpdir) / "00000000000000bb";

  const auto broker = std::make_shared<BrokerMock>();

  EXPECT_CALL(*broker, connect(Connection{journal, 1, {}, {}}))
    .Times(1)
    .WillOnce(Return(Connection{
      broker, 1, Clock{{0xBB, 1}},
      IdConnectionInfoMap{{0xBB, {1, Clock{{0xBB, 1}}}}}
    }));
  EXPECT_CALL(*broker, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}}}));

  journal->connect(broker);

  std::ifstream file(journal->filename());
  EXPECT_EQ(file.peek(), std::ifstream::traits_type::eof());

  EXPECT_TRUE(fs::exists(bbFilename));
  std::string line;
  getline(std::ifstream(bbFilename), line);
  EXPECT_EQ(line, "{{{bb, 1}}, {bb, 10, {}}}");
}
