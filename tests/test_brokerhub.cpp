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

#include "brokerhub.h"

using namespace Cashmere;

using ::testing::AtLeast;
using ::testing::Return;

class BrokerMock : public BrokerI
{
public:
  MOCK_METHOD(Id, id, (), (const, override));
  MOCK_METHOD(
    Clock, insert, (const EntryList& entries, Port sender), (override)
  );
  MOCK_METHOD(Clock, insert, (const Entry& data, Port sender), (override));
  MOCK_METHOD(EntryList, entries, (const Clock& from), (const, override));
  MOCK_METHOD(
    EntryList, entries, (const Clock& from, Port ignore), (const, override)
  );
  MOCK_METHOD(IdConnectionInfoMap, provides, (Port to), (const, override));
  MOCK_METHOD(IdClockMap, versions, (), (const, override));
  MOCK_METHOD(Clock, clock, (), (const, override));
  MOCK_METHOD(Port, connect, (BrokerIPtr other), (override));
  MOCK_METHOD(Port, disconnect, (Port port), (override));
  MOCK_METHOD(BrokerIPtr, ptr, (), (override));
  MOCK_METHOD(void, setClock, (const Clock& clock), (override));
  MOCK_METHOD(
    void, connect, (BrokerIPtr source, Port local, Port remote), (override)
  );
  MOCK_METHOD(Port, getLocalPortFor, (BrokerIPtr broker), (override));
  MOCK_METHOD(Port, getLocalPortFor, (Connection conn), (override));
  MOCK_METHOD(std::set<Port>, connectedPorts, (), (const, override));
};

TEST(BrokerHub, StartsWithNoConnections)
{
  BrokerHubPtr broker = std::make_shared<BrokerHub>();
  EXPECT_EQ(broker->connectedPorts(), std::set<Port>{});
}

TEST(BrokerHub, BrokerFirstConnectionUsesPortOne)
{
  BrokerHubPtr broker = std::make_shared<BrokerHub>();

  EXPECT_EQ(broker->connect(std::make_shared<BrokerMock>()), 1);
  EXPECT_EQ(broker->connectedPorts(), std::set<Port>{1});
}

TEST(BrokerHub, BrokerHubForwardsInserts)
{
  BrokerHubPtr hub = std::make_shared<BrokerHub>();

  auto aa = std::make_shared<BrokerMock>();
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, insert(entry, 0)).Times(1);

  EXPECT_EQ(hub->connect(aa), 1);

  hub->insert(entry, 0);
}

TEST(BrokerHub, BrokerHubOnlyForwardsInsertsToPortsDifferentOfTheSender)
{
  BrokerHubPtr hub = std::make_shared<BrokerHub>();

  auto aa = std::make_shared<BrokerMock>();
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, insert(entry, 1)).Times(0);

  const Port port = hub->connect(aa);

  EXPECT_EQ(port, 1);

  hub->insert(entry, port);
}

TEST(BrokerHub, BrokeHubConnectionsAreFullDuplex)
{
  BrokerHubPtr hub0 = std::make_shared<BrokerHub>();
  BrokerHubPtr hub1 = std::make_shared<BrokerHub>();
  const auto aa = std::make_shared<BrokerMock>();

  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, insert(entry, /* hub1Port */ 1)).Times(1);
  EXPECT_CALL(*aa, getLocalPortFor((Connection{hub0, /* aaPort */ 2})))
    .Times(1)
    .WillOnce(Return(1));

  const Port hub1Port = hub0->connect(hub1);
  const Port aaPort = hub0->connect(aa);

  EXPECT_EQ(hub1Port, 1);
  EXPECT_EQ(aaPort, 2);

  EXPECT_EQ(hub0->connectedPorts(), std::set<Port>({1, 2}));
  EXPECT_EQ(hub1->connectedPorts(), std::set<Port>{1});

  hub1->insert(entry, 0);
}
