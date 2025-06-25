// Cashmere - a distributed conflict free replicated database.
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
#ifndef BARK_OPTIONS_BARK
#define BARK_OPTIONS_BARK

#include <string>
#include <unordered_map>

using OptionsMap = std::unordered_map<std::string, std::string>;

class Options
{
public:
  Options(int argc, char* argv[]);

  bool contains(const std::string& option) const;
  size_t size() const;

  static OptionsMap parse(int argc, char* argv[]);

private:
  OptionsMap _options;
};

#endif
