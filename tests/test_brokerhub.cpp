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
  MOCK_METHOD(bool, connect, (BrokerIPtr other), (override));
  MOCK_METHOD(bool, disconnect, (Port port), (override));
  MOCK_METHOD(BrokerIPtr, ptr, (), (override));
  MOCK_METHOD(void, setClock, (const Clock& clock), (override));
  MOCK_METHOD(
    void, connect, (BrokerIPtr source, Port local, Port remote), (override)
  );
  MOCK_METHOD(Port, getLocalPortFor, (BrokerIPtr broker), (override));
};

TEST(BrokerHub, BrokerConnects)
{
  BrokerHubPtr broker = std::make_shared<BrokerHub>();

  EXPECT_TRUE(broker->connect(std::make_shared<BrokerMock>()));
}
