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
#include "journalfile.h"
#include "test/common.h"

#include <cassert>
#include <filesystem>
#include <signal/signal.h>

namespace fs = std::filesystem;

namespace Cashmere
{
constexpr Id kFixtureId = 0xbaadcafe;
constexpr char const* kFixtureIdStr = "00000000baadcafe";

struct JournalFileFixture
{
  JournalFileFixture()
    : tmpdir(CreateTempDir())
    , filename(fs::path(tmpdir) / kFixtureIdStr)
  {
    journal = std::make_shared<JournalFile>(kFixtureId, tmpdir);
  }
  ~JournalFileFixture()
  {
    DeleteTempDir(tmpdir);
  }
  const std::string tmpdir;
  const std::string filename;
  JournalFilePtr journal;
};

struct JournalMock : public Journal
{
  JournalMock(Id id)
    : JournalMock(id, {})
  {
  }

  JournalMock(Id id, ClockDataMap e)
    : Journal(id, e)
  {
    _insertArgs.clear();
    _queryArgs.clear();
    _entriesSignaler.connect([this](const Clock& clock) -> bool {
      _queryArgs.push_back(clock);
      return false;
    });
  }
  EntryList query(const Clock& from = {}, Port sender = 0) const override
  {
    _entriesSignaler(from);
    return Journal::query(from, sender);
  }
  Clock insert(const Entry& data, Port sender = 0) override
  {
    _insertArgs.push_back(data);
    return Journal::insert(data, sender);
  }
  Signaller::Signal<void(const Clock&)> _entriesSignaler;
  EntryList _insertArgs = {};
  ClockList _queryArgs = {};
};

using JournalMockPtr = std::shared_ptr<JournalMock>;

struct SingleEntryMock : public JournalMock
{
  SingleEntryMock()
    : SingleEntryMock(0xAA, 10)
  {
  }

  SingleEntryMock(Id id, Amount amount)
    : JournalMock(id, {{Clock{{id, 1}}, Data{id, amount, Clock{}}}})
  {
    assert((clock() == Clock{{id, 1}}));
    assert(entries().size() == 1);
  }
};

struct BrokerWithEmptyMock
{
  BrokerPtr broker0 = std::make_shared<Broker>();
  JournalMockPtr aa = std::make_shared<JournalMock>(0xAA);
};

struct BrokerWithAttachedEmptyMockAndOneEmpty : BrokerWithEmptyMock
{
  BrokerWithAttachedEmptyMockAndOneEmpty()
    : BrokerWithEmptyMock()
  {
    broker0->connect(connected);
  }
  JournalMockPtr connected = std::make_shared<JournalMock>(0xFF);
};

struct BrokerWithSingleEntryMock
{
  BrokerPtr broker0 = std::make_shared<Broker>();
  JournalMockPtr aa = std::make_shared<SingleEntryMock>(0xAA, 1);
};

struct BrokerWithAttachedSingleEntryMock : public BrokerWithSingleEntryMock
{

  BrokerWithAttachedSingleEntryMock()
  {
    broker0->connect(aa);
  }
  JournalMockPtr aa = std::make_shared<SingleEntryMock>(0xAA, 1);
};

struct BrokerAndTwoSingleEntryMocks
{
  BrokerPtr broker0 = std::make_shared<Broker>();
  JournalMockPtr aa = std::make_shared<SingleEntryMock>(0xAA, 1);
  JournalMockPtr bb = std::make_shared<SingleEntryMock>(0xBB, 2);
};

struct BrokerWithTwoAttachedSingleEntryMocks
  : public BrokerAndTwoSingleEntryMocks
{
  BrokerWithTwoAttachedSingleEntryMocks()
  {
    broker0->connect(aa);
    broker0->connect(bb);
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

struct TwoBrokerWithASingleEntryMocksEach : BrokerAndTwoSingleEntryMocks
{
  TwoBrokerWithASingleEntryMocksEach()
  {
    broker0->connect(aa);
    broker1->connect(bb);
  }
  BrokerPtr broker1 = std::make_shared<Broker>();
};
}
#endif
