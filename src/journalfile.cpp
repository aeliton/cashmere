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

std::string Filename(const std::string& base, Id id)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(sizeof(Id) * 2) << id;
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

JournalFile::JournalFile(const std::string& directory)
  : JournalBase()
  , _filename(Filename(directory, id()))
  , _lineCount(LineCount(_filename))
{
}

JournalFile::JournalFile(Id id, const std::string& directory)
  : JournalBase(id)
  , _filename(Filename(directory, this->id()))
  , _lineCount(LineCount(_filename))
{
}

JournalFile::~JournalFile() {}

bool JournalFile::save(const Entry& data)
{
  return true;
}

Data JournalFile::entry(Clock time) const
{
  return {};
}

EntryList JournalFile::entries() const
{
  EntryList list;
  return list;
}

std::string JournalFile::filename() const
{
  return _filename;
}

}
