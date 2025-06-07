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
#ifndef CASHMERE_TEST_FIXTURES_H
#define CASHMERE_TEST_FIXTURES_H

#include "broker.h"
#include <cassert>

namespace Cashmere
{
struct JournalMock : public JournalBase
{
  JournalMock(Id id)
    : JournalMock(id, {})
  {
  }
  JournalMock(Id id, ClockEntryList e)
    : JournalBase(id)
    , _entries(e)
  {
    if (e.size() > 0) {
      setClock(e.front().clock);
      assert(clock() == e.front().clock);
    }
    _entriesSignaler.connect([this](const Clock& clock) -> bool {
      _entriesArgs.push_back(clock);
      return false;
    });
  }
  ClockEntryList entries(const Clock& from = {}) const override
  {
    _entriesSignaler(from);
    return _entries;
  }
  virtual bool insert(const ClockEntry& data) override
  {
    _insertArgs.push_back(data);
    _entries.push_back(data);
    setClock(clock().merge(data.clock));
    return true;
  }
  ClockEntryList _entries;
  Signal<void(const Clock&)> _entriesSignaler;
  ClockEntryList _insertArgs = {};
  ClockList _entriesArgs = {};
};

using JournalMockPtr = std::shared_ptr<JournalMock>;

struct SingleEntryMock : public JournalMock
{
  SingleEntryMock()
    : SingleEntryMock(0xAA, 10)
  {
  }

  SingleEntryMock(Id id, Amount amount)
    : JournalMock(id, {{Clock{{id, 1}}, Entry{id, amount, Clock{}}}})
  {
    assert((clock() == Clock{{id, 1}}));
    assert(entries().size() == 1);
  }
};

struct BrokerWithEmptyMock
{
  Broker broker;
  JournalMockPtr mock = std::make_shared<JournalMock>(0xAA);
};

struct BrokerWithSingleEntryMock
{
  Broker broker;
  JournalMockPtr mock = std::make_shared<SingleEntryMock>(0xAA, 1);
};

struct BrokerWithTwoSingleEntryMocks
{
  Broker broker;
  JournalMockPtr aa = std::make_shared<SingleEntryMock>(0xAA, 1);
  JournalMockPtr bb = std::make_shared<SingleEntryMock>(0xBB, 2);
};

struct BrokerWithTwoAttachedSingleEntryMocks
  : public BrokerWithTwoSingleEntryMocks
{
  BrokerWithTwoAttachedSingleEntryMocks()
  {
    broker.attach(aa);
    broker.attach(bb);
  }
};

struct BrokerWithTwoAttachedSingleEntryAndOneEmpty
  : public BrokerWithTwoAttachedSingleEntryMocks
{
  BrokerWithTwoAttachedSingleEntryAndOneEmpty()
    : BrokerWithTwoAttachedSingleEntryMocks()
  {
  }
  JournalMockPtr cc = std::make_shared<JournalMock>(0xCC);
};
}
#endif
