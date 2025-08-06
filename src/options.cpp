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

#include <cassert>
#include <sstream>
#include <unistd.h>

Options::Options() = default;

Options::Options(int argc, char* argv[])
{
  int opt;
  optind = 1;
  while (_error.status == Status::Ok &&
         (opt = getopt(argc, argv, "d:i:h:p:s")) != -1) {
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
      case 'd':
        dbPath = optarg;
        break;
      case 'p':
        try {
          port = std::stoi(optarg);
        } catch (const std::exception&) {
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
      if (optind < argc) {
        std::stringstream ss;
        for (; optind < argc; optind++) {
          ss << argv[optind] << " ";
        }
        command = Command::Read(ss);
      } else if (!service) {
        _error.status = Status::MissingCommand;
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

bool Options::ok() const
{
  return _error.status == Status::Ok;
}
