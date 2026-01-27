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

#include "core/storeimpl.h"
#include "utils/fileutils.h"

using testing::ElementsAre;
using testing::ResultOf;

using namespace Cashmere;

std::string CallSchema(const std::pair<std::string, BrokerCreator>& pair)
{
  return pair.second("")->schema();
}

TEST(Plugin, LoadPlugins)
{
  SchemaFunctorMap plugins = BrokerStore::Impl::LoadPlugins(InstallDirectory() / "lib/cashmere/plugins");
  EXPECT_THAT(plugins,
    ElementsAre(
      ResultOf(CallSchema, "cache"),
      ResultOf(CallSchema, "file"),
      ResultOf(CallSchema, "grpc"),
      ResultOf(CallSchema, "hub")
    )
  );
}
