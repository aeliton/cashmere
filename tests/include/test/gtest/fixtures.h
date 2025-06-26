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
#ifndef CASHMERE_TESTS_GTEST_FIXTURES_H
#define CASHMERE_TESTS_GTEST_FIXTURES_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

#include "journalfile.h"

using ::testing::Return;

namespace fs = std::filesystem;

namespace Cashmere
{

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

}

#endif
