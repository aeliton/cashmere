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
#include "journalfile.h"
#include <catch2/catch_all.hpp>
#include <filesystem>

using namespace Cashmere;

namespace fs = std::filesystem;

constexpr Id kFixtureId = 0xbaadcafe;
constexpr char const* kFixtureIdStr = "00000000baadcafe";

struct JournalFileFixture
{
  JournalFileFixture()
    : tmpdir(fs::temp_directory_path() / std::to_string(Random{}.next()))
    , filename(fs::path(tmpdir) / kFixtureIdStr)
  {
    assert(!fs::exists(tmpdir));
    journal = std::make_shared<JournalFile>(kFixtureId, tmpdir);
  }
  ~JournalFileFixture()
  {
    const fs::path directory = fs::path(tmpdir);
    assert(fs::exists(directory));
    [[maybe_unused]] const bool deleted = fs::remove_all(directory);
    assert(deleted);
  }
  const std::string tmpdir;
  const std::string filename;
  JournalFilePtr journal;
};

TEST_CASE_METHOD(JournalFileFixture, "file creation", "[journalfile]")
{
  SECTION("filename is the hex representation of the id")
  {
    REQUIRE(journal->filename() == filename);
  }
}

SCENARIO_METHOD(JournalFileFixture, "append entries", "[journalfile]")
{
  GIVEN("an empty JournalFile")
  {
    WHEN("inserting an entry")
    {
      journal->append(10);
      THEN("the entry is appended to the the file")
      {
        std::ifstream file(filename);
        std::string line;
        std::getline(file, line);
        REQUIRE(line == "{{{baadcafe, 1}}, {baadcafe, 10, {}}}");
      }
      AND_WHEN("adding a second entry")
      {
        journal->append(20);
        THEN("the second entry is added on the second line")
        {
          std::ifstream file(filename);
          std::string line;
          std::getline(file, line);
          std::getline(file, line);
          REQUIRE(line == "{{{baadcafe, 2}}, {baadcafe, 20, {}}}");
        }
      }
    }
  }
}
