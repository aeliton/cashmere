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
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using isbuf_it = std::istreambuf_iterator<char>;

namespace Cashmere
{
constexpr char kLF = '\n';

std::fstream& SeekToLine(std::fstream& file, size_t line)
{
  file.seekg(std::ios::beg);
  for (size_t i = 0; i < line - 1; ++i) {
    file.ignore(std::numeric_limits<std::streamsize>::max(), kLF);
  }
  return file;
}

std::string Filename(const std::string& base, Id id)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(sizeof(Id) * 2) << id
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
  return std::count(isbuf_it(file), isbuf_it(), kLF);
}

JournalFile::JournalFile(const std::string& location)
  : JournalBase()
  , _location(location)
{
}

JournalFile::JournalFile(Id id, const std::string& location)
  : JournalBase(id)
  , _location(location)
{
}

JournalFile::~JournalFile() {}

bool JournalFile::save(const Entry& data)
{
  std::ofstream file(
    Filename(_location, data.entry.id), std::ios::binary | std::ios::app
  );
  file << data << std::endl;
  return true;
}

Data JournalFile::entry(Clock clock) const
{
  for (const auto& [id, count] : clock) {
    std::fstream file(Filename(_location, id), std::ios::binary | std::ios::in);
    SeekToLine(file, clock.at(id));
    Entry entry;
    Entry::Read(file, entry);
    if (entry.clock == clock) {
      return entry.entry;
    }
  }
  return {};
}

EntryList JournalFile::entries() const
{
  EntryList list;
  for (const auto& [id, count] : clock()) {
    std::fstream file(Filename(_location, id), std::ios::binary | std::ios::in);
    for (size_t i = 0; i < count; ++i) {
      Entry entry;
      if (Entry::Read(file, entry)) {
        list.push_back(entry);
      }
      file.get();
    }
  }
  return list;
}

std::string JournalFile::filename() const
{
  return Filename(_location, id());
}

}
