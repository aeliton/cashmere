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
#include "test/catch2/fixtures.h"
#include <catch2/catch_all.hpp>
#include <fstream>

using namespace Cashmere;

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

TEST_CASE_METHOD(JournalFileWithEntriesFixture, "entries retrieval")
{
  SECTION("third entry")
  {
    const auto clock = entries.at(2).clock;
    REQUIRE(journal->entry(clock) == entries.at(2).entry);
  }
  SECTION("the first from another journal")
  {
    const auto clock = entries.at(3).clock;
    REQUIRE(journal->entry(clock) == entries.at(3).entry);
  }
}

TEST_CASE_METHOD(
  JournalFileWithEntriesFixture, "entries retrieval", "[entries]"
)
{
  const EntryList list = {
    {{{0xBB, 1}}, {0xBB, 100, {}}},
    {{{kFixtureId, 1}}, {kFixtureId, 10, {}}},
    {{{kFixtureId, 2}}, {kFixtureId, 20, {}}},
    {{{kFixtureId, 3}}, {kFixtureId, 30, {}}}
  };
  REQUIRE(journal->entries() == list);
}
