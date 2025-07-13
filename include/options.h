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

#include "entry.h"
#include <string>

struct Command
{
  enum class Type
  {
    Unknown,
    Connect,
    Disconnect,
    Append,
    Relay,
    Sources,
    Versions,
    ListCommands,
    Quit
  };
  std::istream& read(std::istream& in);

  std::string name() const;

  Type type = Type::Unknown;
  std::string url;
  Cashmere::Port port;
  Cashmere::Data data = {};

private:
  std::string _name;
};

struct Options
{
  enum class Status
  {
    Ok,
    InvalidArgument,
    InvalidOptionArgument,
    MissingCommand,
    MissingCommandArgument,
    MissingOption
  };

  struct Error
  {
    Status status = Status::Ok;
    char option;
    std::string optionArgument;
  };

  explicit Options();
  explicit Options(int argc, char* argv[]);

  Error error() const;

  bool ok() const
  {
    return _error.status == Status::Ok;
  }

  Cashmere::Id id = 0;
  Cashmere::Port port = 54321;
  std::string hostname = "0.0.0.0";
  bool service = false;
  Command command = {};

private:
  Error _error;
};

bool operator==(const Command& a, const Command& b);
bool operator==(const Options& a, const Options& b);
bool operator==(const Options::Error& a, const Options::Error& b);

#endif
