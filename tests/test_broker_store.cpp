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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cashmere/broker.h"
#include "cashmere/brokergrpc.h"
#include "cashmere/journalfile.h"
#include "core.h"

using namespace Cashmere;

TEST(BrokerStore, RetrieveInstance)
{
  ASSERT_TRUE(BrokerStore::instance());
}

TEST(BrokerStore, CanCreateHubTypeBrokers)
{
  auto hub = BrokerStore::instance()->build("hub://");
  ASSERT_TRUE(dynamic_cast<Broker*>(hub.get()));
}

TEST(BrokerStore, UseIdFromUrl)
{
  auto hub = BrokerStore::instance()->build("hub://baadcafe@localhost");
  ASSERT_EQ(hub->id(), 0xbaadcafe);
}

TEST(BrokerStore, ReturnNullptrForUnknownSchema)
{
  auto instance = BrokerStore::instance()->build("_unknown_schema_");
  ASSERT_EQ(instance, nullptr);
}

TEST(BrokerStore, CanCreateJournalType)
{
  auto instance = BrokerStore::instance()->build("file:///tmp");
  ASSERT_TRUE(dynamic_cast<JournalFile*>(instance.get()));
}

TEST(BrokerStore, CanCreateGrpcType)
{
  auto instance = BrokerStore::instance()->build("grpc://0.0.0.0:9999");
  ASSERT_TRUE(dynamic_cast<BrokerGrpc*>(instance.get()));
}
