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

#include "cashmere/journalfile.h"
#include "utils/fileutils.h"

namespace fs = std::filesystem;

namespace Cashmere
{
constexpr Id kFixtureId = 0xbaadcafe;
constexpr char const* kFixtureIdStr = "00000000baadcafe";

struct JournalFileTest : public ::testing::Test
{
  JournalFileTest()
    : ::testing::Test()
    , tmpdir(CreateTempDir())
    , filename(fs::path(tmpdir) / kFixtureIdStr)
    , journal(std::make_shared<JournalFile>(kFixtureId, tmpdir))
  {
  }
  virtual ~JournalFileTest()
  {
    DeleteTempDir(tmpdir);
  }
  std::string tmpdir;
  std::string filename;
  JournalFilePtr journal;
};

struct JournalFileWithEntriesTest : public JournalFileTest
{
  JournalFileWithEntriesTest()
    : JournalFileTest()
  {
    for (const auto& entry : entries) {
      journal->insert(entry);
    }
  }
  const std::vector<Entry> entries = {
    {{{kFixtureId, 1}}, {kFixtureId, 10, {}}},
    {{{kFixtureId, 2}}, {kFixtureId, 20, {}}},
    {{{kFixtureId, 3}}, {kFixtureId, 30, {}}},
    {{{0xBB, 1}}, {0xBB, 100, {}}}
  };
};

}
using namespace Cashmere;

using ::testing::Return;

TEST_F(JournalFileTest, FilenameIsTheHexRepresentationOfIdPaddedWithZeroes)
{
  ASSERT_TRUE(journal->filename().ends_with(kFixtureIdStr));
}

TEST_F(JournalFileTest, AppendEntriesToFile)
{
  journal->append(10);
  journal->append(20);
  std::ifstream file(filename);
  std::string line1;
  std::string line2;
  std::getline(file, line1);
  std::getline(file, line2);
  ASSERT_EQ(line1, "{{{baadcafe, 1}}, {baadcafe, 10, {}}}");
  ASSERT_EQ(line2, "{{{baadcafe, 2}}, {baadcafe, 20, {}}}");
}

TEST_F(JournalFileWithEntriesTest, EntriesRetrieval)
{
  ASSERT_EQ(journal->entry(entries.at(2).clock), entries.at(2).entry);
  ASSERT_EQ(journal->entry(entries.at(3).clock), entries.at(3).entry);
}

TEST_F(JournalFileWithEntriesTest, RetrieveAllEntries)
{
  const EntryList list = {
    {{{0xBB, 1}}, {0xBB, 100, {}}},
    {{{kFixtureId, 1}}, {kFixtureId, 10, {}}},
    {{{kFixtureId, 2}}, {kFixtureId, 20, {}}},
    {{{kFixtureId, 3}}, {kFixtureId, 30, {}}}
  };
  ASSERT_EQ(journal->entries(), list);
}

TEST_F(JournalFileTest, SeparateFilesPerJournal)
{
  const std::string bbFilename = fs::path(tmpdir) / "00000000000000bb";

  const auto broker = std::make_shared<BrokerMock>();

  EXPECT_CALL(
    *broker, connect(Connection(
               journal, 1, {}, {{kFixtureId, {.distance = 1, .clock = {}}}}
             ))
  )
    .Times(1)
    .WillOnce(Return(Connection(
      broker, 1, Clock{{0xBB, 1}},
      IdConnectionInfoMap{{0xBB, {1, Clock{{0xBB, 1}}}}}
    )));
  EXPECT_CALL(*broker, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}}}));

  journal->connect(Connection{broker});

  std::ifstream file(journal->filename());
  EXPECT_EQ(file.peek(), std::ifstream::traits_type::eof());

  EXPECT_TRUE(fs::exists(bbFilename));
  std::string line;
  getline(std::ifstream(bbFilename), line);
  EXPECT_EQ(line, "{{{bb, 1}}, {bb, 10, {}}}");
}
