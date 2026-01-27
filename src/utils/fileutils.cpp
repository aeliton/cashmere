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
#include "utils/fileutils.h"
#include "utils/random.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
using isbuf_it = std::istreambuf_iterator<char>;

namespace Cashmere
{

bool ReadSpaces(std::istream& in)
{
  int c = 0;
  while (in.peek() == kSpace) {
    c = in.get();
  }
  return c == kSpace;
}

bool ReadChar(std::istream& in, const char expected)
{
  ReadSpaces(in);
  if (in.peek() == expected) {
    in.get();
    return true;
  }
  return false;
}

bool ReadPair(std::istream& in, uint64_t& id, uint64_t& time)
{
  if (!ReadChar(in, kOpenCurly)) {
    return false;
  }
  in >> std::hex >> id >> std::dec;
  if (in.fail()) {
    return false;
  }
  if (!ReadChar(in, kComma)) {
    return false;
  }
  in >> time;
  if (in.fail()) {
    return false;
  }
  if (!ReadChar(in, kCloseCurly)) {
    return false;
  }
  return true;
}

bool SeekToLine(std::fstream& file, size_t line)
{
  file.seekg(std::ios::beg);
  for (; line > 1 && file.good(); line--) {
    file.ignore(std::numeric_limits<std::streamsize>::max(), kLineFeed);
  }
  return line == 1;
}

std::string Filename(const std::string& base, uint64_t id)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(sizeof(uint64_t) * 2) << id
     << std::dec;
  const auto filename = fs::path(base) / ss.str();
  fs::create_directories(filename.parent_path());
  return filename;
}

size_t LineCount(const std::string& filename)
{
  if (!fs::exists(filename)) {
    return 0;
  }
  std::ifstream file(filename);
  return std::count(isbuf_it(file), isbuf_it(), kLineFeed);
}

fs::path InstallDirectory()
{
  return fs::canonical("/proc/self/exe")
    .parent_path()
    .parent_path();
}

std::vector<std::string> ListFiles(const std::string& path)
{
  std::vector<std::string> found;
  for (const auto& entry : fs::directory_iterator(path)) {
    found.push_back(entry.path());
  }
  return found;
}

std::string CreateTempDir()
{
  std::string tempDir;
  do {
    tempDir = fs::temp_directory_path() / std::to_string(Random{}.next());
  } while (fs::exists(tempDir));
  fs::create_directories(tempDir);
  return tempDir;
}

void DeleteTempDir(std::string tempDir)
{
  assert(fs::exists(tempDir));
  [[maybe_unused]] const bool deleted = fs::remove_all(tempDir);
  assert(deleted);
}

}
