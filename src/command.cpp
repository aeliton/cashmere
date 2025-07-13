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
#include "command.h"

#include <istream>
#include <unordered_map>

static const std::unordered_map<std::string, Command::Type> kNameTypeMap = {
  {"connect", Command::Type::Connect},
  {"disconnect", Command::Type::Disconnect},
  {"add", Command::Type::Append},
  {"relay", Command::Type::Relay},
  {"sources", Command::Type::Sources},
  {"list", Command::Type::ListCommands},
  {"quit", Command::Type::Quit}
};

static const std::unordered_map<Command::Type, std::string> kTypeNameMap = {
  {Command::Type::Unknown, "<unknown>"},
  {Command::Type::Connect, "connect"},
  {Command::Type::Disconnect, "disconnect"},
  {Command::Type::Append, "add"},
  {Command::Type::Relay, "relay"},
  {Command::Type::Sources, "sources"},
  {Command::Type::ListCommands, "list"},
  {Command::Type::Quit, "quit"}
};

std::istream& Command::read(std::istream& in)
{
  *this = {};

  in >> _name;

  const auto it = kNameTypeMap.find(_name);
  if (it == kNameTypeMap.cend()) {
    return in;
  }

  type = it->second;

  std::string argument;
  switch (it->second) {
    case Type::Connect:
      in >> url;
      break;
    case Type::Relay:
      in >> argument;
      data.id = std::stoull(argument);
      [[fallthrough]];
    case Type::Append:
      in >> argument;
      data.value = std::stoul(argument);
      break;
    case Type::Disconnect:
      in >> argument;
      port = std::stoi(argument);
      break;
    default:
      break;
  }

  return in;
}

std::string Command::name() const
{
  return _name;
}

bool operator==(const Command& a, const Command& b)
{
  return a.type == b.type && a.data == b.data;
}
