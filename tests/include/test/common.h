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
#ifndef CASHMERE_TESTS_COMMON_H
#define CASHMERE_TESTS_COMMON_H

#include "random.h"
#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

namespace Cashmere
{

inline std::string CreateTempDir()
{
  std::string tempDir;
  do {
    tempDir = fs::temp_directory_path() / std::to_string(Random{}.next());
  } while (fs::exists(tempDir));
  fs::create_directories(tempDir);
  return tempDir;
}

inline void DeleteTempDir(std::string tempDir)
{
  assert(fs::exists(tempDir));
  [[maybe_unused]] const bool deleted = fs::remove_all(tempDir);
  assert(deleted);
}

}

#endif
