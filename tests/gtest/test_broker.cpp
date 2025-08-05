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
#include "test/gtest/brokermock.h"

using namespace Cashmere;

using ::testing::Return;

TEST(Broker, StartsWithNoConnections)
{
  BrokerPtr broker = std::make_shared<Broker>();
  EXPECT_EQ(broker->connectedPorts(), std::set<Port>{});
}

TEST(Broker, BrokerFirstConnectionUsesPortOne)
{
  BrokerPtr hub = std::make_shared<Broker>();
  auto aa = std::make_shared<BrokerMock>();

  EXPECT_CALL(*aa, connect((Connection{BrokerStub(hub), 1, {}, {}})))
    .Times(1)
    .WillOnce(Return(Connection{
      BrokerStub{aa}, 1, Clock{}, IdConnectionInfoMap{{0xAA, {0, {}}}}
    }));

  hub->connect(BrokerStub{aa});
  EXPECT_EQ(hub->connectedPorts(), std::set<Port>{1});
}

TEST(Broker, BrokerForwardsInserts)
{
  BrokerPtr hub = std::make_shared<Broker>();

  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  const auto aa = std::make_shared<BrokerMock>();

  EXPECT_CALL(*aa, connect(Connection{BrokerStub(hub), 1, {}, {}}))
    .Times(1)
    .WillOnce(Return(Connection{BrokerStub{aa}, 1, {}, {{0xAA, {0, Clock{}}}}})
    );
  EXPECT_CALL(*aa, insert(entry, 1)).Times(1);

  hub->connect(BrokerStub{aa});
  hub->insert(entry, 0);
}

TEST(Broker, BrokerOnlyForwardsInsertsToPortsDifferentOfTheSender)
{
  BrokerPtr hub = std::make_shared<Broker>();

  auto aa = std::make_shared<BrokerMock>();
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, connect(Connection{BrokerStub(hub), 1, {}, {}}))
    .Times(1)
    .WillOnce(Return(Connection{BrokerStub{aa}, 1, {}, {{0xAA, {0, {}}}}}));
  EXPECT_CALL(*aa, insert(entry, 1)).Times(0);

  const auto conn = hub->connect(BrokerStub{aa});

  EXPECT_EQ(conn.port(), 1);

  hub->insert(entry, conn.port());
}

TEST(Broker, BrokeHubConnectionsAreFullDuplex)
{
  BrokerPtr hub0 = std::make_shared<Broker>();
  BrokerPtr hub1 = std::make_shared<Broker>();
  const auto aa = std::make_shared<BrokerMock>();

  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, insert(entry, /* hub1Port */ 1)).Times(1);
  EXPECT_CALL(*aa, connect((Connection{BrokerStub(hub0), 2, {}, {}})))
    .Times(1)
    .WillOnce(Return(Connection{BrokerStub{aa}, 1, {}, {{0xAA, {0, {}}}}}));

  const auto hub1Conn = hub0->connect(BrokerStub{hub1});
  const auto aaConn = hub0->connect(BrokerStub{aa});

  EXPECT_EQ(hub1Conn.port(), 1);
  EXPECT_EQ(aaConn.port(), 2);

  EXPECT_EQ(hub0->connectedPorts(), std::set<Port>({1, 2}));
  EXPECT_EQ(hub1->connectedPorts(), std::set<Port>{1});

  hub1->insert(entry, 0);
}

TEST(Broker, UpdatesItsClockDuringInsert)
{
  BrokerPtr hub = std::make_shared<Broker>();
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};
  hub->insert(entry, 0);
  EXPECT_EQ(hub->clock(), entry.clock);
}

TEST(Broker, UpdatesItsClockDuringConnect)
{
  BrokerPtr hub = std::make_shared<Broker>();
  const auto aa = std::make_shared<BrokerMock>();
  const Clock aaClock = Clock{{0xAA, 1}};
  EXPECT_CALL(*aa, connect((Connection{BrokerStub(hub), 1, {}, {}})))
    .Times(1)
    .WillOnce(
      Return(Connection{BrokerStub{aa}, 1, aaClock, {{0xAA, {0, aaClock}}}})
    );
  EXPECT_CALL(*aa, query(Clock({}), /* aaPort */ 1))
    .Times(1)
    .WillOnce(Return(EntryList{{aaClock, Data{0xAA, 10, {}}}}));

  hub->connect(BrokerStub{aa});

  EXPECT_EQ(hub->clock(), aaClock);
}

TEST(Broker, UpdateConnectionProvidedSourcesOnAttach)
{
  BrokerPtr hub = std::make_shared<Broker>();
  const auto aa = std::make_shared<BrokerMock>();

  const auto aaClock = Clock{{0xAA, 1}};
  const auto aaEntry = Entry{aaClock, Data{0xAA, 10, {}}};

  EXPECT_CALL(*aa, connect((Connection{BrokerStub(hub), 1, {}, {}})))
    .Times(1)
    .WillOnce(
      Return(Connection{BrokerStub{aa}, 1, aaClock, {{0xAA, {0, aaClock}}}})
    );
  EXPECT_CALL(*aa, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));

  hub->connect(BrokerStub{aa});
}

TEST(Broker, ExchangeEntriesOnConnect)
{
  BrokerPtr hub = std::make_shared<Broker>();
  const auto aa = std::make_shared<BrokerMock>();
  const auto bb = std::make_shared<BrokerMock>();

  const auto aaEntry = Entry{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}};
  const auto bbEntry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 20, {}}};

  EXPECT_CALL(*aa, connect((Connection{BrokerStub(hub), 1, {}, {}})))
    .Times(1)
    .WillOnce(Return(Connection{
      BrokerStub{aa}, 1, Clock{{0xAA, 1}},
      IdConnectionInfoMap{{0xAA, {1, {{0xAA, 1}}}}}
    }));
  EXPECT_CALL(*aa, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));
  EXPECT_CALL(
    *aa, refresh(
           Connection{
             BrokerStub{hub}, 1, Clock{{0xAA, 1}, {0xBB, 1}},
             IdConnectionInfoMap{
               {0xBB, ConnectionInfo{2, Clock{{0xAA, 1}, {0xBB, 1}}}}
             }
           },
           1
         )
  )
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*aa, query(Clock{{0xBB, 1}}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));
  EXPECT_CALL(*aa, insert(bbEntry, 1))
    .Times(1)
    .WillOnce(Return(Clock{{0xAA, 1}, {0xBB, 1}}));

  EXPECT_CALL(
    *bb,
    connect((Connection{
      BrokerStub(hub), 2, Clock{{0xAA, 1}},
      IdConnectionInfoMap{{0xAA, {.distance = 2, .version = Clock{{0xAA, 1}}}}}
    }))
  )
    .Times(1)
    .WillOnce(Return(Connection{
      BrokerStub{bb}, 1, Clock{{0xBB, 1}},
      IdConnectionInfoMap{{0xBB, {.distance = 1, .version = Clock{{0xBB, 1}}}}}
    }));
  EXPECT_CALL(*bb, insert(EntryList{aaEntry}, 1))
    .Times(1)
    .WillOnce(Return(Clock{{0xAA, 1}, {0xBB, 1}}));
  EXPECT_CALL(*bb, query(Clock{{0xAA, 1}}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{bbEntry}));

  hub->connect(BrokerStub{aa});
  hub->connect(BrokerStub{bb});

  EXPECT_EQ(
    hub->provides(),
    IdConnectionInfoMap(
      {{0xAA,
        ConnectionInfo{.distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}}},
       {0xBB,
        ConnectionInfo{.distance = 1, .version = Clock{{0xAA, 1}, {0xBB, 1}}}}}
    )
  );
  EXPECT_EQ(
    hub->versions(),
    IdClockMap(
      {{0xAA, Clock{{0xAA, 1}, {0xBB, 1}}}, {0xBB, Clock{{0xAA, 1}, {0xBB, 1}}}}
    )
  );
}

TEST(Broker, PropagatesProvidedConnections)
{
  const auto broker = std::make_shared<Broker>();
  const auto hub = std::make_shared<BrokerMock>();
  const auto journal = std::make_shared<BrokerMock>();

  const auto aaEntry = Entry{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}};

  EXPECT_CALL(*hub, connect((Connection{BrokerStub(broker), 1, {}, {}})))
    .Times(1)
    .WillOnce(Return(Connection{BrokerStub{hub}, 1, {}, {}}));
  EXPECT_CALL(*hub, query(Clock{}, 1)).Times(1).WillOnce(Return(EntryList{}));
  EXPECT_CALL(*hub, insert(aaEntry, 1))
    .Times(1)
    .WillOnce(Return(Clock{{0xAA, 1}}));
  EXPECT_CALL(
    *hub,
    refresh(
      Connection{
        BrokerStub{broker}, 1, Clock{{0xAA, 1}},
        IdConnectionInfoMap{{0xAA, {.distance = 2, .version = {{0xAA, 1}}}}}
      },
      1
    )
  )
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_CALL(*journal, connect((Connection{BrokerStub(broker), 2, {}, {}})))
    .Times(1)
    .WillOnce(Return(Connection{
      BrokerStub{journal}, 1, Clock{{0xAA, 1}},
      IdConnectionInfoMap{{0xAA, {1, {{0xAA, 1}}}}}
    }));
  EXPECT_CALL(*journal, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));

  broker->connect(BrokerStub{hub});
  broker->connect(BrokerStub{journal});
}
