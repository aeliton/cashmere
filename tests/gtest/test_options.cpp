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
#include "options.h"
#include <gtest/gtest.h>

class Args
{
public:
  Args(std::vector<std::string> arguments)
    : argc(arguments.size())
    , argv(new char*[argc + 1]())
  {
    for (int i = 0; i < argc; i++) {
      argv[i] = strdup(arguments[i].c_str());
    }
    argv[argc] = nullptr;
  }
  ~Args()
  {
    for (int i = 0; i < argc; i++) {
      delete argv[i];
    }
    delete[] argv;
  }

  int argc;
  char** argv;
};

TEST(OptionsParse, NoOptionsOrArgumentsPrintsHelp)
{
  Args args({"anyname"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options, Options{});
  EXPECT_EQ(options.error().status, Options::Status::MissingCommand);
}

TEST(OptionsParse, IdOptionArgumentShouldBeHexadecimal)
{
  Args args({"anyname", "-i", "cafe", "-s"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options.id, 0xCAFE);
  EXPECT_TRUE(options.ok());
}

TEST(OptionsParse, IdRemainsUnitializedWithInvalidOptionArgument)
{
  Args args({"anyname", "-i", "-p"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options.id, 0x00);
  auto error = Options::Error{
    .status = Options::Status::InvalidOptionArgument,
    .option = 'i',
    .optionArgument = "-p"
  };
  EXPECT_EQ(options.error(), error);
}

TEST(OptionsParse, ParsePort)
{
  Args args({"anyname", "-p", "1024", "-s"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options.port, 1024);
  EXPECT_TRUE(options.ok());
}

TEST(OptionsParse, PortRemainsUnitializedWithInvalidOptionArgument)
{
  Args args({"anyname", "-p", "cafe"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options.id, 0x00);
  auto error = Options::Error{
    .status = Options::Status::InvalidOptionArgument,
    .option = 'p',
    .optionArgument = "cafe"
  };
  EXPECT_EQ(options.error(), error);
}

TEST(OptionsParse, ParseHostname)
{
  Args args({"anyname", "-h", "cafe", "-s"});
  Options options(args.argc, args.argv);
  ASSERT_EQ(options.hostname, "cafe");
  EXPECT_TRUE(options.ok());
}

TEST(OptionsParse, ParseAddCommandsWithoutClock)
{
  Args args({"anyname", "add", "10"});
  Options options(args.argc, args.argv);
  const auto data = Cashmere::Data{0x00, 10, {}};
  ASSERT_EQ(options.command.data, data);
  EXPECT_TRUE(options.ok());
}
