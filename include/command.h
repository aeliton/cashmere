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
#ifndef CASHMERE_COMMAND_H
#define CASHMERE_COMMAND_H

#include "entry.h"
#include <string>

struct Command
{
  enum class Type
  {
    Invalid,
    Connect,
    Disconnect,
    Append,
    Relay,
    Sources,
    Versions,
    ListCommands,
    Quit
  };
  static Command Read(std::istream& in);

  std::string name() const;
  bool ok() const;

  Type type = Type::Invalid;
  std::string url;
  Cashmere::Port port;
  Cashmere::Data data = {};

private:
  std::string _name;
};

bool operator==(const Command& a, const Command& b);

bool ReadValueAndOptionalClock(std::istream& in, Cashmere::Data& data);
bool ReadData(std::istream& in, Cashmere::Data& data);

#endif
