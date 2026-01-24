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

#include "brokermock.h"
#include "core.h"

using namespace Cashmere;

using ::testing::Return;

struct BrokerTest : public ::testing::Test
{
  void SetUp() override {
    store = std::make_shared<BrokerStore>();
    hub0 = store->build("hub://");
  }
  BrokerStorePtr store;
  BrokerBasePtr hub0;
};

TEST_F(BrokerTest, ConnectIgnoresNullptr)
{
  const auto conn = hub0->connect(Connection{});
  ASSERT_EQ(conn.source(), -1);
}

TEST_F(BrokerTest, DisconnectedBrokerHasEmptyClock)
{
  ASSERT_EQ(hub0->clock(), Clock{});
}

TEST_F(BrokerTest, DisconnectedBrokerHasEmptyVersions)
{
  ASSERT_EQ(hub0->versions(), IdClockMap{});
}

TEST_F(BrokerTest, SuccessfulConnectionReturnsValidConnection)
{
  auto aa = std::make_shared<BrokerMock>();

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, Clock{}, IdConnectionInfoMap{})));

  const auto conn = hub0->connect(Connection{aa});
  ASSERT_TRUE(conn.valid());
}

TEST_F(BrokerTest, BrokerFirstConnectionUsesPortOne)
{
  auto aa = std::make_shared<BrokerMock>();

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(
      Return(Connection(aa, 1, Clock{}, IdConnectionInfoMap{{0xAA, {0, {}}}}))
    );

  auto conn = hub0->connect(Connection{aa});
  EXPECT_EQ(conn.source(), 1);
}

TEST_F(BrokerTest, BrokerForwardsInserts)
{
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  const auto aa = std::make_shared<BrokerMock>();

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, {}, {{0xAA, {0, Clock{}}}})));
  EXPECT_CALL(*aa, insert(entry, 1)).Times(1);

  hub0->connect(Connection{aa});
  hub0->insert(entry, 0);
}

TEST_F(BrokerTest, BrokerOnlyForwardsInsertsToPortsDifferentOfTheSender)
{
  auto aa = std::make_shared<BrokerMock>();
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, {}, {{0xAA, {0, {}}}})));
  EXPECT_CALL(*aa, insert(entry, 1)).Times(0);

  const auto conn = hub0->connect(Connection{aa});

  EXPECT_EQ(conn.source(), 1);

  hub0->insert(entry, conn.source());
}

TEST_F(BrokerTest, BrokeHubConnectionsAreFullDuplex)
{
  auto hub1 = store->build("hub://");
  const auto aa = std::make_shared<BrokerMock>();

  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};

  EXPECT_CALL(*aa, insert(entry, /* hub1Port */ 1)).Times(1);
  EXPECT_CALL(*aa, connect(Connection(hub0, 2, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, {}, {{0xAA, {0, {}}}})));

  const auto hub1Conn = hub0->connect(Connection{hub1});
  const auto aaConn = hub0->connect(Connection{aa});

  EXPECT_EQ(hub1Conn.source(), 1);
  EXPECT_EQ(aaConn.source(), 2);

  EXPECT_EQ(hub0->connectedPorts(), std::set<Source>({1, 2}));
  EXPECT_EQ(hub1->connectedPorts(), std::set<Source>{1});

  hub1->insert(entry, 0);
}

TEST_F(BrokerTest, UpdatesItsClockDuringInsert)
{
  const auto entry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 10, {}}};
  hub0->insert(entry, 0);
  EXPECT_EQ(hub0->clock(), entry.clock);
}

TEST_F(BrokerTest, UpdatesItsClockDuringConnect)
{
  const auto aa = std::make_shared<BrokerMock>();
  const Clock aaClock = Clock{{0xAA, 1}};
  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, aaClock, {{0xAA, {1, aaClock}}})));
  EXPECT_CALL(*aa, query(Clock({}), /* aaPort */ 1))
    .Times(1)
    .WillOnce(Return(EntryList{{aaClock, Data{0xAA, 10, {}}}}));

  hub0->connect(Connection{aa});

  const IdClockMap expectedVersion{{0xAA, aaClock}};
  EXPECT_EQ(hub0->versions(), expectedVersion);
  const SourcesMap expectedSources{{1, {{0xAA, {1, Clock{{0xAA, 1}}}}}}};
  EXPECT_EQ(hub0->sources(), expectedSources);
  ASSERT_EQ(hub0->clock(), aaClock);
}

TEST_F(BrokerTest, VersionsArePreservedAfterDisconnection)
{
  const auto aa = std::make_shared<BrokerMock>();
  const Clock aaClock = Clock{{0xAA, 1}};
  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, aaClock, {{0xAA, {1, aaClock}}})));
  EXPECT_CALL(*aa, query(Clock({}), 1))
    .Times(1)
    .WillOnce(Return(EntryList{{aaClock, Data{0xAA, 10, {}}}}));

  hub0->connect(Connection{aa});
  const Source source = hub0->disconnect(1);
  EXPECT_EQ(source, 1);

  EXPECT_EQ(hub0->sources(), SourcesMap{});
  EXPECT_EQ(hub0->clock(), aaClock);

  const IdClockMap expectedVersion{{0xAA, aaClock}};
  ASSERT_EQ(hub0->versions(), expectedVersion);
}

TEST_F(BrokerTest, UpdateConnectionProvidedSourcesOnAttach)
{
  const auto aa = std::make_shared<BrokerMock>();

  const auto aaClock = Clock{{0xAA, 1}};
  const auto aaEntry = Entry{aaClock, Data{0xAA, 10, {}}};

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(aa, 1, aaClock, {{0xAA, {0, aaClock}}})));
  EXPECT_CALL(*aa, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));

  hub0->connect(Connection{aa});
}

TEST_F(BrokerTest, ExchangeEntriesOnConnect)
{
  const auto aa = std::make_shared<BrokerMock>();
  const auto bb = std::make_shared<BrokerMock>();

  const auto aaEntry = Entry{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}};
  const auto bbEntry = Entry{Clock{{0xBB, 1}}, Data{0xBB, 20, {}}};

  EXPECT_CALL(*aa, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(
      aa, 1, Clock{{0xAA, 1}}, IdConnectionInfoMap{{0xAA, {1, {{0xAA, 1}}}}}
    )));
  EXPECT_CALL(*aa, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));
  EXPECT_CALL(
    *aa, refresh(
           Connection(
             hub0, 1, Clock{{0xAA, 1}, {0xBB, 1}},
             IdConnectionInfoMap{
               {0xBB, ConnectionInfo{2, Clock{{0xAA, 1}, {0xBB, 1}}}}
             }
           ),
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
    connect(Connection(
      hub0, 2, Clock{{0xAA, 1}},
      IdConnectionInfoMap{{0xAA, {.distance = 2, .clock = Clock{{0xAA, 1}}}}}
    ))
  )
    .Times(1)
    .WillOnce(Return(Connection(
      bb, 1, Clock{{0xBB, 1}},
      IdConnectionInfoMap{{0xBB, {.distance = 1, .clock = Clock{{0xBB, 1}}}}}
    )));
  EXPECT_CALL(*bb, insert(EntryList{aaEntry}, 1))
    .Times(1)
    .WillOnce(Return(Clock{{0xAA, 1}, {0xBB, 1}}));
  EXPECT_CALL(*bb, query(Clock{{0xAA, 1}}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{bbEntry}));

  hub0->connect(Connection{aa});
  hub0->connect(Connection{bb});

  EXPECT_EQ(
    hub0->sources(),
    SourcesMap(
      {{1,
        IdConnectionInfoMap(
          {{0xAA,
            ConnectionInfo{.distance = 1, .clock = Clock{{0xAA, 1}, {0xBB, 1}}}}
          }
        )},
       {2,
        IdConnectionInfoMap(
          {{0xBB,
            ConnectionInfo{.distance = 1, .clock = Clock{{0xAA, 1}, {0xBB, 1}}}}
          }
        )}}
    )
  );
  ASSERT_EQ(
    hub0->versions(),
    IdClockMap(
      {{0xAA, Clock{{0xAA, 1}, {0xBB, 1}}}, {0xBB, Clock{{0xAA, 1}, {0xBB, 1}}}}
    )
  );
}

TEST_F(BrokerTest, PropagatesProvidedConnections)
{
  const auto hub = std::make_shared<BrokerMock>();
  const auto journal = std::make_shared<BrokerMock>();

  const auto aaEntry = Entry{Clock{{0xAA, 1}}, Data{0xAA, 10, {}}};

  EXPECT_CALL(*hub, connect(Connection(hub0, 1, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(hub, 1, {}, {})));
  EXPECT_CALL(*hub, query(Clock{}, 1)).Times(1).WillOnce(Return(EntryList{}));
  EXPECT_CALL(*hub, insert(aaEntry, 1))
    .Times(1)
    .WillOnce(Return(Clock{{0xAA, 1}}));
  EXPECT_CALL(
    *hub, refresh(
            Connection(
              hub0, 1, Clock{{0xAA, 1}},
              IdConnectionInfoMap{{0xAA, {.distance = 2, .clock = {{0xAA, 1}}}}}
            ),
            1
          )
  )
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_CALL(*journal, connect(Connection(hub0, 2, {}, {})))
    .Times(1)
    .WillOnce(Return(Connection(
      journal, 1, Clock{{0xAA, 1}},
      IdConnectionInfoMap{{0xAA, {1, {{0xAA, 1}}}}}
    )));
  EXPECT_CALL(*journal, query(Clock{}, 1))
    .Times(1)
    .WillOnce(Return(EntryList{aaEntry}));

  hub0->connect(Connection{hub});
  hub0->connect(Connection{journal});
}
