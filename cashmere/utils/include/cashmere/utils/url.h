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
#ifndef CASHMERE_UTILS_URL_H
#define CASHMERE_UTILS_URL_H

#include <string>
#include <ostream>

namespace Cashmere
{

struct Url
{
  std::string url;
  std::string schema;
  std::string id;
  std::string hostport;
  std::string path;

  bool valid() const;
  auto operator<=>(const Url&) const = default;
};

Url ParseUrl(const std::string& url);

std::ostream& operator<<(std::ostream& os, const Url& data);

}

#endif
