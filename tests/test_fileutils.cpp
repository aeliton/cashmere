// Cashmere - a distributed conflict-free replicated database.
// Copyright (C) 2026 Aeliton G. Silva
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "utils/fileutils.h"

constexpr char const* kPluginsPath = "lib/cashmere/plugins";
using testing::EndsWith;

using namespace Cashmere;

TEST(FileUtils, FindPluginPaths)
{
  std::vector<std::string> found = ListFiles(InstallDirectory() / kPluginsPath);
  EXPECT_THAT(found,
    UnorderedElementsAre(
      EndsWith("libcache.so"),
      EndsWith("libfile.so"),
      EndsWith("libgrpc.so"),
      EndsWith("libhub.so"))
  );
}
