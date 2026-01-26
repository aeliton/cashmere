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

#include "brokergrpcstub.h"
#include "cashmere/broker.h"
#include "cashmere/journalfile.h"
#include "core.h"

using namespace Cashmere;

struct BrokerStoreTest : public ::testing::Test
{
  void SetUp() override {
    store = BrokerStore::create();
  }
  BrokerStorePtr store;
};

TEST_F(BrokerStoreTest, CanCreateHubTypeBrokers)
{
  auto hub = store->getOrCreate("hub://");
  ASSERT_TRUE(dynamic_cast<Broker*>(hub.get()));
}

TEST_F(BrokerStoreTest, UseIdFromUrl)
{
  auto hub = store->getOrCreate("hub://baadcafe@localhost");
  ASSERT_EQ(hub->id(), 0xbaadcafe);
}

TEST_F(BrokerStoreTest, ReturnNullptrForUnknownSchema)
{
  auto instance = store->getOrCreate("_unknown_schema_");
  ASSERT_EQ(instance, nullptr);
}

TEST_F(BrokerStoreTest, CanCreateJournalType)
{
  auto instance = store->getOrCreate("file:///tmp");
  ASSERT_TRUE(dynamic_cast<JournalFile*>(instance.get()));
}

TEST_F(BrokerStoreTest, CanCreateGrpcType)
{
  auto instance = store->getOrCreate("grpc://0.0.0.0:9999");
  ASSERT_TRUE(dynamic_cast<BrokerGrpcStub*>(instance.get()));
}

TEST_F(BrokerStoreTest, CreatedBrokerHasWeakPtrToStore)
{
  auto hub = store->getOrCreate("hub://");
  ASSERT_EQ(hub->store(), store);
}

TEST_F(BrokerStoreTest, GetCreatesBrokerIfInexisting)
{
  store->getOrCreate("hub://aa@localhost");
  ASSERT_EQ(store->size(), 1);
}

TEST_F(BrokerStoreTest, GetPreviouslyCreatedBroker)
{
  store->getOrCreate("hub://aa@localhost");
  const auto retrieved = store->getOrCreate("hub://aa@localhost");
  ASSERT_EQ(retrieved->url(), "hub://aa@localhost");
}
