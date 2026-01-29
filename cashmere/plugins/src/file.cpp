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
#include "file.h"
#include "cashmere/utils/file.h"
#include <fstream>

namespace Cashmere
{

JournalFile::JournalFile(const std::string& url)
  : JournalBase(url)
{
}

JournalFile::~JournalFile() {}

bool JournalFile::save(const Entry& data)
{
  std::ofstream file(
    Filename(location(), data.entry.id), std::ios::binary | std::ios::app
  );
  file << data << std::endl;
  return true;
}

Data JournalFile::entry(Clock clock) const
{
  for (const auto& [id, count] : clock) {
    std::fstream file(Filename(location(), id), std::ios::binary | std::ios::in);
    if (!SeekToLine(file, count)) {
      break;
    }
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
    std::fstream file(Filename(location(), id), std::ios::binary | std::ios::in);
    for (size_t i = 0; i < count; ++i) {
      Entry entry;
      if (Entry::Read(file, entry)) {
        list.push_back(entry);
      }
      ReadChar(file, kLineFeed);
    }
  }
  return list;
}

std::string JournalFile::filename() const
{
  return Filename(location(), id());
}


BrokerBase* JournalFile::create(const std::string& url)
{
  return new JournalFile(url);
}

std::string JournalFile::schema() const
{
  return "file";
}

extern "C" CASHMERE_EXPORT Cashmere::BrokerBase* create(const std::string& url)
{
  return new Cashmere::JournalFile(url);
}

}
