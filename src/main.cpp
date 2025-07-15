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

void runService(const Options& options);
void runCommand(const Options& options);

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

  if (options.service) {
    runService(options);
  } else {
    runCommand(options);
  }

  return 0;
}

void runCommand(const Options& options)
{
  auto stub = BrokerGrpcStub(options.hostname, options.port);
  switch (options.command.type) {
    case Command::Type::Invalid:
      break;
    case Command::Type::Connect:
      std::cout << "connect to " << options.command.url << std::endl;
      stub.connect(ConnectionData(BrokerStub(options.command.url)));
      break;
    case Command::Type::Disconnect:
      break;
    case Command::Type::Append:
      if (!stub.relay(options.command.data, 0).valid()) {
        std::cerr << "Error: failed inserting:" << options.command.data
                  << options.hostname << std::endl;
        exit(EXIT_FAILURE);
      }
      break;
    case Command::Type::Relay:
      if (!stub.relay(options.command.data, 0).valid()) {
        std::cerr << "Error: failed inserting:" << options.command.data
                  << options.hostname << std::endl;
        exit(EXIT_FAILURE);
      }
      break;
    case Command::Type::Sources:
      break;
    case Command::Type::Versions:
      break;
    case Command::Type::ListCommands:
      break;
    case Command::Type::Quit:
      break;
  }
}

void runService(const Options& options)
{
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

  if (options.command.type == Command::Type::Append) {
    broker->relay(options.command.data, 0);
  }

  Command command;
  while (command.type != Command::Type::Quit) {
    std::cout << journal->clock() << "[" << Ledger::Balance(journal->entries())
              << "]"
              << "> ";
    command = Command::Read(std::cin);

    switch (command.type) {
      case Command::Type::Invalid:
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
        command.data.id = journal->id();
        journal->append(command.data);
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
}
