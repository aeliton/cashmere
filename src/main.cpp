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
#include "brokerbase.h"
#include "brokergrpc.h"
#include "brokergrpcstub.h"
#include "journalfile.h"
#include "ledger.h"
#include "options.h"
#include "utils/fileutils.h"

#include <grpcpp/server.h>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace Cashmere;

int main(int argc, char* argv[])
{
  Options options(argc, argv);
  if (!options.ok()) {
    std::cerr
      << "usage: %s  [-s] [-h hostname] [-p port] [-i id] [<command>...]"
      << std::endl
      << "status: " << static_cast<int>(options.error().status) << std::endl;
    exit(EXIT_FAILURE);
  }

  if (!options.service) {
    auto stub = BrokerGrpcStub(options.hostname, options.port);
    auto clock = stub.clock();
    if (clock.valid()) {
      Entry entry{clock.tick(options.id), options.command.data};
      if (options.command.type == Command::Type::Append) {
        auto clock = stub.relay(entry, 0);
        std::cerr << "requesting to " << options.hostname << ":" << options.port
                  << " to relay [insert " << entry << "]; response: " << clock
                  << std::endl;
      }
    } else {
      std::cerr << "Error: failed retrieving version from " << options.hostname
                << ":" << options.port << std::endl;
      exit(EXIT_FAILURE);
    }
    return 0;
  }

  auto tempDir = TempDir();
  auto broker = std::make_shared<BrokerGrpc>(options.hostname, options.port);
  auto journal = std::make_shared<JournalFile>(options.id, tempDir.directory);

  journal->connect(BrokerStub{broker});

  std::cout << "Journal " << std::hex << options.id << std::dec
            << " port: " << options.port << " path: " << tempDir.directory
            << std::endl;

  auto grpcBroker = broker->start();

  std::thread brokerThread([&grpcBroker]() { grpcBroker->Wait(); });

  brokerThread.detach();

  Command command;
  while (command.type != Command::Type::Quit) {
    std::cout << journal->clock() << "[" << Ledger::Balance(journal->entries())
              << "]"
              << "> ";

    command.read(std::cin);
    switch (command.type) {
      case Command::Type::Unknown:
        std::cerr << "unknown command: " << command.name() << std::endl;
        break;
      case Command::Type::Connect:
        if (broker->connect(BrokerStub(command.url)) < 0) {
          std::cerr << command.name() << ": failed [" << options.port << "]"
                    << std::endl;
        }
        break;
      case Command::Type::Disconnect:
        broker->disconnect(command.port);
        break;
      case Command::Type::Append:
        journal->append(command.data.value);
        break;
      case Command::Type::Relay:
        std::cerr << "not implemented" << std::endl;
        break;
      case Command::Type::Sources:
        std::cout << journal->provides() << std::endl;
        break;
      case Command::Type::Versions:
        std::cout << journal->versions() << std::endl;
        break;
      case Command::Type::ListCommands:
        std::cerr << "not implemented" << std::endl;
        break;
      case Command::Type::Quit:
        std::cout << "bye!" << std::endl;
        break;
    }
  }

  grpcBroker->Shutdown();

  return 0;
}
