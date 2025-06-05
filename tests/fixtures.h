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
    : JournalMock(id, {}, {})
  {
  }
  JournalMock(Id id, Clock c, ClockEntryList e)
    : _id(id)
    , _clock(c)
    , _entries(e)
  {
    _entriesSignaler.connect([this](const Clock& clock) {
      _entriesArgs.push_back(clock);
    });
  }
  const Id id() const override
  {
    return _id;
  }
  Clock clock() const override
  {
    return _clock;
  }
  ClockEntryList entries(const Clock& from = {}) const override
  {
    _entriesSignaler(from);
    return _entries;
  }
  virtual bool insert(const Clock& clock, const Entry& entry) override
  {
    _insertArgs.push_back({clock, entry});
    return false;
  }
  ClockChangeSignal& clockChanged() override
  {
    return _signal;
  }
  const Id _id;
  const Clock _clock;
  const ClockEntryList _entries;
  ClockChangeSignal _signal;
  Signal<void(const Clock&)> _entriesSignaler;
  ClockEntryList _insertArgs = {};
  ClockList _entriesArgs = {};
};

using JournalMockPtr = std::shared_ptr<JournalMock>;

struct SingleEntry : public JournalMock
{
  SingleEntry(Id id, Amount amount)
    : JournalMock(id, {{id, 1}}, {{Clock{{id, 1}}, Entry{id, amount, Clock{}}}})
  {
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

struct TwoAttachedSingleEntryOneEmpty : public TwoSingleEntryMocks
{
  TwoAttachedSingleEntryOneEmpty()
  {
    broker.attach(aa);
    broker.attach(bb);
  }
  JournalMockPtr cc = std::make_shared<JournalMock>(0xCC);
};

}
#endif
