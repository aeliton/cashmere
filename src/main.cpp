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
#include "random.h"
#include "utils/fileutils.h"

#include <grpcpp/server.h>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace Cashmere;

int main(int argc, char* argv[])
{
  int opt;
  uint16_t port = 5432;
  uint64_t id = Random{}.next();
  std::string hostname = "0.0.0.0";
  bool idProvided = false;
  bool server = false;
  while ((opt = getopt(argc, argv, "i:h:p:s")) != -1) {
    switch (opt) {
      case 'i':
      {
        idProvided = true;
        std::stringstream ss(optarg);
        ss >> std::hex >> id >> std::dec;
        break;
      }
      case 'h':
        hostname = std::string(optarg);
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 's':
        server = true;
        break;
      default:
        fprintf(
          stderr,
          "Usage: %s  [-s] [-h hostname] [-p port] [-i id] [<command>...]\n",
          argv[0]
        );
        exit(EXIT_FAILURE);
    }
  }

  if (!server) {
    if (!idProvided) {
      std::cout << "-i <id> and a command are required in command mode";
      return EXIT_FAILURE;
    }
    auto stub = std::make_shared<BrokerGrpcStub>(hostname, port);
    std::cout << "requesting to " << hostname << ":" << port
              << " to relay [insert 500]; response: "
              << stub->relay(Entry{{{0xAA, 1}}, {0xAA, 500, {}}}, 0)
              << std::endl;

    return 0;
  }

  auto tempDir = TempDir();
  auto broker = std::make_shared<BrokerGrpc>("0.0.0.0", port);
  auto journal = std::make_shared<JournalFile>(id, tempDir.directory);

  journal->connect(BrokerStub{broker});

  std::cout << "Journal " << std::hex << id << std::dec << " port: " << port
            << " path: " << tempDir.directory << std::endl;

  auto grpcBroker = broker->start();

  std::thread brokerThread([&grpcBroker]() { grpcBroker->Wait(); });

  brokerThread.detach();

  std::string command;
  while (true) {
    std::cout << journal->clock() << "[" << Ledger::Balance(journal->entries())
              << "]"
              << "> ";

    std::cin >> command;

    if (command == "add") {
      Amount value;
      std::cin >> value;
      if (std::cin.fail()) {
        std::cerr << "add error: not a numeric value" << std::endl;
        continue;
      }
      journal->append(value);
    } else if (command == "connect") {
      std::string url;
      std::cin >> url;
      Port success = broker->connect(BrokerStub(url));
      if (success < 0) {
        std::cerr << command << ": failed [" << port << "]" << std::endl;
      }
    } else if (command == "provides") {
      std::cout << journal->provides() << std::endl;
    } else if (command == "versions") {
      std::cout << journal->versions() << std::endl;
    } else if (command == "disconnect") {
      Port port;
      std::cin >> port;
      broker->disconnect(port);
    } else if (command == "q" || command == "quit") {
      std::cout << "bye!" << std::endl;
      break;
    } else {
      std::cerr << "unknown command: " << command << std::endl;
    }
  }

  grpcBroker->Shutdown();

  return 0;
}
