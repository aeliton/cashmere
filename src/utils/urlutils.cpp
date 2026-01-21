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
#include "urlutils.h"

#include <regex>

namespace Cashmere
{

constexpr char const* kRegexStr = "([^:]*):/{2}(([^/][^@]*)@){0,1}([^/]*){0,1}(/.*){0,1}";
Url ParseUrl(const std::string& url)
{
  auto regex = std::regex(kRegexStr);
  std::smatch matches;
  std::regex_search(url, matches, regex);
  if (matches.size() > 5) {
    return {url, matches[1], matches[3], matches[4], matches[5]};
  }
  return {url, "", "", "", ""};
}

bool Url::valid() const
{
  return url.size() > 0 && schema.size() > 0 && (id.size() > 0 || hostport.size() > 0);
}

std::ostream& operator<<(std::ostream& os, const Url& data)
{
  return os << "{" << data.url << ", " << data.id << ", " << data.hostport
            << ", " << data.path << "}";
}

}
