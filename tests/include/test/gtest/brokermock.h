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
#ifndef CASHMERE_GTESTS_BROKERMOCK_H
#define CASHMERE_GTESTS_BROKERMOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "brokerbase.h"

namespace Cashmere
{

class BrokerMock : public BrokerBase
{
public:
  MOCK_METHOD(Id, id, (), (const, override));
  MOCK_METHOD(Clock, insert, (const Entry& data, Port sender), (override));
  MOCK_METHOD(
    EntryList, query, (const Clock& from, Port sender), (const, override)
  );
  MOCK_METHOD(IdConnectionInfoMap, provides, (Port to), (const, override));
  MOCK_METHOD(IdClockMap, versions, (), (const, override));
  MOCK_METHOD(Clock, clock, (), (const, override));
  MOCK_METHOD(Port, disconnect, (Port port), (override));
  MOCK_METHOD(BrokerBasePtr, ptr, (), (override));
  MOCK_METHOD(void, setClock, (const Clock& clock), (override));
  MOCK_METHOD(ConnectionData, connect, (Connection conn), (override));
  MOCK_METHOD(
    bool, refresh, (const ConnectionData& data, Port port), (override)
  );
  MOCK_METHOD(std::set<Port>, connectedPorts, (), (const, override));
};

}

#endif
