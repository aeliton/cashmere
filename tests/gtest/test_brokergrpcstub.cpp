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
#include "cashmere/journal.h"
#include "test/gtest/brokermock.h"
#include <proto/cashmere_mock.grpc.pb.h>

using namespace ::Cashmere;
using namespace ::testing;

constexpr int32_t kPort = 10;

using StubInterfacePtr = std::unique_ptr<Grpc::Broker::StubInterface>;

TEST(BrokerGrpcStub, StartsConnectionsUsingGrpcStub)
{
  auto broker = std::make_shared<Broker>();

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  Grpc::ConnectionResponse resp;
  resp.set_port(kPort);
  (*resp.mutable_version())[0xBB] = 1;

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
      _, ResultOf([](Grpc::QueryRequest in) { return in.sender(); }, Eq(kPort)),
      _
    )
  )
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(queryResponse), Return(grpc::Status::OK)));

  auto brokerStub = BrokerStub{
    std::make_shared<BrokerGrpcStub>(std::move(stub)), BrokerStub::Type::Grpc
  };

  broker->connect(brokerStub);
  EXPECT_EQ(broker->clock(), Clock({{0xBB, 1}}));
}

TEST(BrokerGrpcStub, InsertIsCalledPassingTheCorrectPort)
{
  auto journal = std::make_shared<Journal>(0xAA);
  journal->append(1000);

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  Grpc::ConnectionResponse resp;
  resp.set_port(kPort);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  Grpc::InsertResponse response;
  (*response.mutable_version())[0xAA] = 1;

  EXPECT_CALL(
    *stub,
    Insert(
      _,
      ResultOf([](Grpc::InsertRequest in) { return in.sender(); }, Eq(kPort)), _
    )
  )
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(response), Return(grpc::Status::OK)));

  auto brokerStub = BrokerStub{
    std::make_shared<BrokerGrpcStub>(std::move(stub)), BrokerStub::Type::Grpc
  };

  journal->connect(brokerStub);
}

TEST(BrokerGrpcStub, InsertIsCalledOnConnect)
{
  auto journal = std::make_shared<Journal>(0xAA);
  journal->append(1000);

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  Grpc::InsertResponse response;
  (*response.mutable_version())[0xAA] = 1;

  EXPECT_CALL(*stub, Insert(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(response), Return(grpc::Status::OK)));

  auto brokerStub = BrokerStub{
    std::make_shared<BrokerGrpcStub>(std::move(stub)), BrokerStub::Type::Grpc
  };

  journal->connect(brokerStub);
}

TEST(BrokerGrpcStub, RefreshIsCalledOnDisconnect)
{
  auto journal = std::make_shared<Broker>();

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  Grpc::ConnectionResponse resp;
  resp.set_port(10);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  EXPECT_CALL(*stub, Refresh(_, _, _))
    .Times(1)
    .WillOnce(Return(grpc::Status::OK));

  auto brokerStub = BrokerStub{
    std::make_shared<BrokerGrpcStub>(std::move(stub)), BrokerStub::Type::Grpc
  };

  journal->connect(brokerStub);
  EXPECT_EQ(journal->disconnect(1), 1);
}

TEST(BrokerGrpcStub, RefreshIsCalledWithSender)
{
  auto broker = std::make_shared<Broker>();

  auto other = std::make_shared<BrokerMock>();
  broker->connect(BrokerStub{other});

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  Grpc::ConnectionResponse resp;
  resp.set_port(kPort);

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));

  EXPECT_CALL(
    *stub,
    Refresh(
      _,
      ResultOf([](Grpc::RefreshRequest in) { return in.sender(); }, Eq(kPort)),
      _
    )
  )
    .Times(1)
    .WillOnce(Return(grpc::Status::OK));

  auto brokerStub = BrokerStub{
    std::make_shared<BrokerGrpcStub>(std::move(stub)), BrokerStub::Type::Grpc
  };

  broker->connect(brokerStub);
  EXPECT_TRUE(broker->refresh(Connection(), 1));
}
