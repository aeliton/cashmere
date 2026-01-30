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
#ifndef CASHMERE_UTILS_FILEUTILS_H
#define CASHMERE_UTILS_FILEUTILS_H

#include <istream>
#include <filesystem>
#include <vector>
#include <cashmere/cashmere_export.h>

namespace Cashmere
{
constexpr char kSpace = ' ';
constexpr char kComma = ',';
constexpr char kOpenCurly = '{';
constexpr char kCloseCurly = '}';
constexpr char kLineFeed = '\n';

std::string CASHMERE_EXPORT CreateTempDir();

void CASHMERE_EXPORT DeleteTempDir(std::string tempDir);

bool CASHMERE_EXPORT ReadSpaces(std::istream& in);

bool CASHMERE_EXPORT ReadChar(std::istream& in, const char expected);

bool CASHMERE_EXPORT ReadPair(std::istream& in, uint64_t& id, uint64_t& time);

bool CASHMERE_EXPORT SeekToLine(std::fstream& file, size_t line);

std::string CASHMERE_EXPORT Filename(const std::string& base, uint64_t id);

size_t CASHMERE_EXPORT LineCount(const std::string& filename);

std::filesystem::path CASHMERE_EXPORT InstallDirectory();

std::vector<std::string> CASHMERE_EXPORT ListFiles(const std::string& path);

struct CASHMERE_EXPORT  TempDir
{
  TempDir()
    : directory(CreateTempDir())
  {
  }
  ~TempDir()
  {
    DeleteTempDir(directory);
  }
  std::string directory;
};

}

#endif
