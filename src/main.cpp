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
#include "cashmere/brokerbase.h"
#include "cashmere/brokergrpc.h"
#include "cashmere/brokergrpcclient.h"
#include "cashmere/journalfile.h"
#include "cashmere/ledger.h"
#include "options.h"

#include <cstdlib>
#include <filesystem>
#include <grpcpp/server.h>
#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>

#include <editline/history.h>
#include <editline/readline.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace
{
struct TempDir
{
  TempDir();
  ~TempDir();
  std::string directory;
};
}

using namespace Cashmere;

namespace lg = spdlog;

void RunService(const Options& options);
void RunCommand(const Options& options);

int main(int argc, char* argv[])
{
  lg::set_default_logger(lg::stderr_color_mt("log"));

  Options options(argc, argv);
  if (!options.ok()) {
    std::cout << "usage: " << argv[0]
              << " [-s] [-h hostname] [-p port] [-i id] [<command>...]"
              << std::endl;
    exit(EXIT_FAILURE);
  }

  if (options.service) {
    RunService(options);
  } else {
    RunCommand(options);
  }

  return 0;
}

void RunCommand(const Options& options)
{
  auto stub = BrokerGrpcClient(options.hostname, options.port);
  switch (options.command.type) {
    case Command::Type::Invalid:
      break;
    case Command::Type::Connect:
      lg::info("connect to {}", options.command.url);
      stub.connect(Connection(BrokerStub(options.command.url)));
      break;
    case Command::Type::Disconnect:
      break;
    case Command::Type::Append:
      if (!stub.relay(options.command.data, 0).valid()) {
        lg::error("add command failed!");
        exit(EXIT_FAILURE);
      }
      break;
    case Command::Type::Relay:
      if (!stub.relay(options.command.data, 0).valid()) {
        lg::error("relay command failed!");
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

void RunService(const Options& options)
{
  auto tempDir = TempDir();
  auto broker = std::make_shared<BrokerGrpc>(options.hostname, options.port);
  auto journal = std::make_shared<JournalFile>(
    options.id, options.dbPath.empty() ? tempDir.directory : options.dbPath
  );

  journal->connect(BrokerStub{broker});

  lg::info(
    "Journal {:x}, port: {}, path: {}", options.id, options.port,
    tempDir.directory
  );

  std::thread brokerThread = broker->start();

  brokerThread.detach();

  if (options.command.type == Command::Type::Append) {
    journal->append(options.command.data.value);
  }

  Command command;
  while (command.type != Command::Type::Quit) {
    std::stringstream ss;
    ss << journal->clock() << "[" << Ledger::Balance(journal->entries()) << "]"
       << "> ";
    char* line = readline(ss.str().c_str());
    if (!line) {
      break;
    }
    add_history(line);
    std::stringstream in(line);

    command = Command::Read(in);
    free(line);

    switch (command.type) {
      case Command::Type::Invalid:
        std::cout << "unknown command: " << command.name() << std::endl;
        break;
      case Command::Type::Connect:
      {
        const auto conn = broker->connect(BrokerStub(command.url));
        if (!conn.valid()) {
          std::cout << command.name() << ": failed [" << options.port << "]"
                    << std::endl;
        }
        break;
      }
      case Command::Type::Disconnect:
        broker->disconnect(command.port);
        break;
      case Command::Type::Append:
        command.data.id = journal->id();
        journal->append(command.data);
        break;
      case Command::Type::Relay:
        broker->relay(command.data, 0);
        break;
      case Command::Type::Sources:
        std::cout << journal->provides() << std::endl;
        break;
      case Command::Type::Versions:
        std::cout << journal->versions() << std::endl;
        break;
      case Command::Type::ListCommands:
        std::cout << "not implemented" << std::endl;
        break;
      case Command::Type::Quit:
        std::cout << "bye!" << std::endl;
        break;
    }
  }

  broker->stop();
}

namespace fs = std::filesystem;

namespace
{

TempDir::TempDir()
{
  std::mt19937_64 engine;
  std::uniform_int_distribution<uint64_t> distribution;

  do {
    directory =
      fs::temp_directory_path() / std::to_string(distribution(engine));
  } while (fs::exists(directory));
  fs::create_directories(directory);
}

TempDir::~TempDir()
{
  assert(fs::exists(directory));
  [[maybe_unused]] const bool deleted = fs::remove_all(directory);
  assert(deleted);
}

}
