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
#include "utils/fileutils.h"

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
  {Command::Type::Invalid, "<unknown>"},
  {Command::Type::Connect, "connect"},
  {Command::Type::Disconnect, "disconnect"},
  {Command::Type::Append, "add"},
  {Command::Type::Relay, "relay"},
  {Command::Type::Sources, "sources"},
  {Command::Type::ListCommands, "list"},
  {Command::Type::Quit, "quit"}
};

Command Command::Read(std::istream& in)
{
  Command command = {};

  in >> command._name;

  const auto it = kNameTypeMap.find(command._name);
  if (it == kNameTypeMap.cend()) {
    return command;
  }

  command.type = it->second;

  std::string argument;
  switch (it->second) {
    case Type::Connect:
      in >> command.url;
      break;
    case Type::Relay:
      in >> argument;
      if (!ReadData(in, command.data)) {
        command.type = Type::Invalid;
      }
      [[fallthrough]];
    case Type::Append:
      if (!ReadValueAndOptionalClock(in, command.data)) {
        command.type = Type::Invalid;
      }
      break;
    case Type::Disconnect:
      in >> argument;
      command.port = std::stoi(argument);
      break;
    default:
      break;
  }

  return command;
}

std::string Command::name() const
{
  return _name;
}

bool operator==(const Command& a, const Command& b)
{
  return a.type == b.type && a.data == b.data;
}

bool Command::ok() const
{
  return type != Type::Invalid;
}

bool ReadValueAndOptionalClock(std::istream& in, Cashmere::Data& data)
{
  in >> data.value;
  if (in.fail()) {
    return false;
  }
  Cashmere::ReadSpaces(in);
  if ((in.peek() == Cashmere::kLineFeed || in.eof())) {
    return true;
  }
  return Cashmere::Clock::Read(in, data.alters);
}

bool ReadData(std::istream& in, Cashmere::Data& data)
{
  in >> std::hex >> data.id >> std::dec;
  if (in.fail()) {
    return false;
  }
  return ReadValueAndOptionalClock(in, data);
}
