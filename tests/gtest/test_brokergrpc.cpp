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

#include "brokergrpc.h"
#include <proto/cashmere_mock.grpc.pb.h>

using namespace ::Cashmere;
using namespace ::testing;

using StubInterfacePtr = std::unique_ptr<Grpc::Broker::StubInterface>;

TEST(BrokerGrpc, StartsConnectionsUsingGrpcStub)
{
  auto broker = std::make_shared<BrokerGrpc>(1000);

  Grpc::Connection resp;
  resp.set_port(10);
  (*resp.mutable_version()->mutable_data())[0xBB] = 1;

  Grpc::EntryList entries;
  auto entry = entries.add_data();
  entry->mutable_data()->set_id(0xBB);
  entry->mutable_data()->set_value(1000);
  (*entry->mutable_clock()->mutable_data())[0xBB] = 1;

  auto stub = std::make_unique<Grpc::MockBrokerStub>();

  EXPECT_CALL(*stub, Connect(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(resp), Return(grpc::Status::OK)));
  EXPECT_CALL(*stub, Query(_, _, _))
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<2>(entries), Return(grpc::Status::OK)));

  auto grpcBrokerStub = std::make_shared<BrokerGrpcStub>(std::move(stub));
  auto brokerStub =
    std::make_shared<BrokerStub>(grpcBrokerStub, BrokerStub::Type::Grpc);

  EXPECT_EQ(broker->connect(brokerStub), 1);
  EXPECT_EQ(broker->clock(), Clock({{0xBB, 1}}));
}
