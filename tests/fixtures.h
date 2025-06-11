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
#include "journal.h"
#include <cassert>

namespace Cashmere
{
struct JournalMock : public Journal
{
  JournalMock(Id id)
    : JournalMock(id, {})
  {
  }
  JournalMock(Id id, ClockEntryList e)
    : Journal(id)
  {
    Broker::insert(e);
    _insertArgs.clear();
    _entriesArgs.clear();
    _entriesSignaler.connect([this](const Clock& clock) -> bool {
      _entriesArgs.push_back(clock);
      return false;
    });
  }
  ClockEntryList entries(const Clock& from = {}) const override
  {
    _entriesSignaler(from);
    return Journal::entries(from);
  }
  bool insert(const ClockEntry& data, Port sender = 0) override
  {
    _insertArgs.push_back(data);
    return Journal::insert(data, sender);
  }
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
  BrokerPtr broker = std::make_shared<Broker>();
  JournalMockPtr mock = std::make_shared<JournalMock>(0xAA);
};

struct BrokerWithAttachedEmptyMockAndOneEmpty : BrokerWithEmptyMock
{
  BrokerWithAttachedEmptyMockAndOneEmpty()
    : BrokerWithEmptyMock()
  {
    broker->attach(attached);
  }
  JournalMockPtr attached = std::make_shared<JournalMock>(0xFF);
};

struct BrokerWithSingleEntryMock
{
  BrokerPtr broker = std::make_shared<Broker>();
  JournalMockPtr mock = std::make_shared<SingleEntryMock>(0xAA, 1);
};

struct BrokerWithAttachedSingleEntryMock : public BrokerWithSingleEntryMock
{

  BrokerWithAttachedSingleEntryMock()
  {
    broker->attach(mock);
  }
  JournalMockPtr mock = std::make_shared<SingleEntryMock>(0xAA, 1);
};

struct BrokerWithTwoSingleEntryMocks
{
  BrokerPtr broker = std::make_shared<Broker>();
  JournalMockPtr aa = std::make_shared<SingleEntryMock>(0xAA, 1);
  JournalMockPtr bb = std::make_shared<SingleEntryMock>(0xBB, 2);
};

struct BrokerWithTwoAttachedSingleEntryMocks
  : public BrokerWithTwoSingleEntryMocks
{
  BrokerWithTwoAttachedSingleEntryMocks()
  {
    broker->attach(aa);
    broker->attach(bb);
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
