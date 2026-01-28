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
#include <proto/cashmere_mock.grpc.pb.h>

#include "cashmere/brokerstore.h"
#include "plugins/grpc.h"

#include "brokermock.h"

using namespace ::Cashmere;
using namespace ::testing;

constexpr int32_t kSource = 10;
constexpr char const* kTestGrpcUrl = "grpc://test:123";

using StubInterfacePtr = std::unique_ptr<Grpc::Broker::StubInterface>;

struct BrokerGrpcStubTest : public ::testing::Test
{
  void SetUp() override {
    store = BrokerStore::create();
    stub = std::make_unique<Grpc::MockBrokerStub>();
  }

  BrokerStoreBasePtr store;
  std::unique_ptr<Grpc::MockBrokerStub> stub;
};

TEST_F(BrokerGrpcStubTest, StartsConnectionsUsingGrpcStub)
{
  auto broker = store->getOrCreate("hub://");

  Grpc::ConnectionResponse resp;
  resp.set_source(kSource);
  (*resp.mutable_clock())[0xBB] = 1;

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  Grpc::QueryResponse queryResponse;
  auto entry = queryResponse.add_entries();
  entry->mutable_data()->set_id(0xBB);
  entry->mutable_data()->set_value(1000);
  (*entry->mutable_clock())[0xBB] = 1;

  EXPECT_CALL(
    *stub,
    Query(
      _,
      ResultOf([](Grpc::QueryRequest in) { return in.sender(); }, Eq(kSource)),
      _
    )
  )
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(queryResponse), Return(grpc::Status::OK)));

  store->insert(kTestGrpcUrl, std::make_shared<BrokerGrpcStub>(std::move(stub)));

  broker->connect(kTestGrpcUrl);
  EXPECT_EQ(broker->clock(), Clock({{0xBB, 1}}));
}

TEST_F(BrokerGrpcStubTest, InsertIsCalledPassingTheCorrectPort)
{
  auto journal = store->getOrCreate("cache://aa");
  journal->append(1000);

  Grpc::ConnectionResponse resp;
  resp.set_source(kSource);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  Grpc::InsertResponse response;
  (*response.mutable_clock())[0xAA] = 1;

  EXPECT_CALL(
    *stub,
    Insert(
      _,
      ResultOf([](Grpc::InsertRequest in) { return in.sender(); }, Eq(kSource)),
      _
    )
  )
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(response), Return(grpc::Status::OK)));

  store->insert(kTestGrpcUrl, std::make_shared<BrokerGrpcStub>(std::move(stub)));

  journal->connect(kTestGrpcUrl);
}

TEST_F(BrokerGrpcStubTest, InsertIsCalledOnConnect)
{
  auto journal = store->getOrCreate("cache://aa@localhost");
  journal->append(1000);

  Grpc::InsertResponse response;
  (*response.mutable_clock())[0xAA] = 1;

  EXPECT_CALL(*stub, Insert(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(response), Return(grpc::Status::OK)));

  store->insert(kTestGrpcUrl, std::make_shared<BrokerGrpcStub>(std::move(stub)));
  journal->connect(kTestGrpcUrl);
}

TEST_F(BrokerGrpcStubTest, RefreshIsCalledOnDisconnect)
{
  auto journal = store->getOrCreate("hub://");

  Grpc::ConnectionResponse resp;
  resp.set_source(10);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  EXPECT_CALL(*stub, Refresh(_, _, _))
    .Times(1)
    .WillOnce(Return(grpc::Status::OK));

  store->insert(kTestGrpcUrl, std::make_shared<BrokerGrpcStub>(std::move(stub)));
  journal->connect(kTestGrpcUrl);
  EXPECT_EQ(journal->disconnect(1), 1);
}

TEST_F(BrokerGrpcStubTest, RefreshIsCalledWithSender)
{
  auto broker = store->getOrCreate("hub://aa@localhost");

  store->insert("hub://bb@localhost", std::make_shared<BrokerMock>());
  broker->connect("hub://bb@localhost");

  Grpc::ConnectionResponse resp;
  resp.set_source(kSource);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  EXPECT_CALL(
    *stub, Refresh(
             _,
             ResultOf(
               [](Grpc::RefreshRequest in) { return in.sender(); }, Eq(kSource)
             ),
             _
           )
  )
    .Times(1)
    .WillOnce(Return(grpc::Status::OK));

  store->insert(kTestGrpcUrl, std::make_shared<BrokerGrpcStub>(std::move(stub)));
  broker->connect(kTestGrpcUrl);
  EXPECT_TRUE(broker->refresh(Connection{}, 1));
}
