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

namespace Cashmere
{
struct JournalMock : public JournalBase
{
  JournalMock(Id id)
    : _id(id)
  {
  }
  JournalMock(Id id, Clock c, JournalEntries e)
    : _id(id)
    , _clock(c)
    , _entries(e)
  {
  }
  const Id id() const override
  {
    return _id;
  }
  Clock clock() const override
  {
    return _clock;
  }
  const JournalEntries& entries() const override
  {
    return _entries;
  }
  virtual bool insert(const Clock& clock, const Entry& entry) override
  {
    ++_insertCount;
    _insertArgs = {clock, entry};
    return false;
  }
  virtual bool append(const Entry& value) override
  {
    return false;
  }
  virtual bool contains(const Clock& clock) const override
  {
    return _entries.find(clock) != _entries.cend();
  }
  ClockChangeSignal& clockChanged() override
  {
    return _signal;
  }
  Id _id;
  Clock _clock;
  JournalEntries _entries;
  ClockChangeSignal _signal;
  size_t _insertCount = 0;
  ClockEntry _insertArgs = {};
};

using JournalMockPtr = std::shared_ptr<JournalMock>;

struct SingleEntry : public JournalMock
{
  SingleEntry(Id id, Amount amount)
    : JournalMock(id, {{id, 1}}, {{Clock{{id, 1}}, Entry{id, amount, Clock{}}}})
  {
  }
  virtual bool append(const Entry&) override
  {
    _signal(_clock, _entries[_clock]);
    return true;
  }
};

struct EmptyMock
{
  Broker broker;
  JournalMockPtr mock = std::make_shared<JournalMock>(0xAA);
};

struct SingleEntryMock
{
  Broker broker;
  JournalMockPtr mock = std::make_shared<SingleEntry>(0xAA, 1);
};

struct TwoSingleEntryMocks
{
  Broker broker;
  JournalMockPtr aa = std::make_shared<SingleEntry>(0xAA, 1);
  JournalMockPtr bb = std::make_shared<SingleEntry>(0xBB, 2);
};

}
#endif
