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
#ifndef CASHEMERE_JOURNAL_H
#define CASHEMERE_JOURNAL_H

#include <cassert>
#include <memory>

#include "broker.h"
#include "random.h"

namespace Cashmere
{

class Journal : public Broker
{
public:
  Journal();
  ~Journal();
  explicit Journal(Id id, const ClockDataMap& entries = {});

  Clock insert(const Entry& data, Port port = 0) override;
  EntryList entries(const Clock& from = {}) const override;
  IdConnectionInfoMap provides(Port to = 0) const override;

  Type type() const override;

  Id id() const;
  const Id bookId() const;
  bool append(Amount value);
  bool append(const Data& entry);
  bool replace(Amount value, const Clock& clock);
  bool erase(Clock time);
  bool contains(const Clock& clock) const;
  Data entry(Clock time) const;

private:
  const Id _id;
  static Random _random;
  const Id _bookId;
  ClockDataMap _entries;
  Clock _version;
};

using JournalPtr = std::shared_ptr<Journal>;
}

#endif
