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
#include "options.h"

#include <sstream>
#include <unistd.h>
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

Options::Options() = default;

Options::Options(int argc, char* argv[])
{
  int opt;
  while (_error.status == Status::Ok &&
         (opt = getopt(argc, argv, "i:h:p:s")) != -1) {
    switch (opt) {
      case 'i':
      {
        std::stringstream ss(optarg);
        ss >> std::hex >> id >> std::dec;
        if (ss.fail()) {
          _error.status = Status::InvalidOptionArgument;
        }
        break;
      }
      case 'p':
        try {
          port = std::stoi(optarg);
        } catch (std::exception) {
          _error.status = Status::InvalidOptionArgument;
        }
        break;
      case 'h':
        hostname = std::string(optarg);
        break;
      case 's':
        service = true;
        break;
      case '?':
        break;
      default:
        break;
    }
  }

  switch (_error.status) {
    case Status::InvalidArgument:
      break;
    case Status::InvalidOptionArgument:
      _error.option = static_cast<char>(opt);
      _error.optionArgument = optarg;
      break;
    case Status::Ok:
      if (!service) {
        if (id == 0) {
          _error.status = Status::MissingOption;
          _error.option = 'i';
        } else if (optind >= argc) {
          _error.status = Status::MissingCommand;
        } else {
          const std::string arg = std::string(argv[optind++]);
          command.type = Command::Type::Append;
          if (arg == "add") {
            if (optind >= argc) {
              _error.status = Status::MissingCommandArgument;
            } else {
              command.data.id = id;
              command.data.value = std::stol(argv[optind++]);
            }
          }
        }
      }
      break;
    default:
      break;
  }
}

Options::Error Options::error() const
{
  return _error;
}

bool operator==(const Command& a, const Command& b)
{
  return a.type == b.type && a.data == b.data;
}

bool operator==(const Options& a, const Options& b)
{
  return a.id == b.id && a.port == b.port && a.hostname == b.hostname &&
         a.service == b.service && a.command == b.command;
}

bool operator==(const Options::Error& a, const Options::Error& b)
{
  return a.status == b.status && a.option == b.option &&
         a.optionArgument == b.optionArgument;
}

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
