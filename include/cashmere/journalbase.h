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
#ifndef CASHEMERE_JOURNAL_BASE_H
#define CASHEMERE_JOURNAL_BASE_H

#include <cassert>
#include <memory>

#include "cashmere/broker.h"

namespace Cashmere
{

class Random;

class CASHMERE_EXPORT JournalBase : public Broker
{
public:
  JournalBase();
  virtual ~JournalBase();
  explicit JournalBase(Id id);

  virtual bool save(const Entry& entry) = 0;
  virtual Data entry(Clock time) const = 0;
  virtual EntryList entries() const = 0;
  SourcesMap sources(Port sender = 0) const override;

  Clock insert(const Entry& data, Port source = 0) override;
  EntryList query(const Clock& from = {}, Port source = 0) const override;
  virtual Clock relay(const Data& data, Port sender) override
  {
    if (data.id == 0) {
      return Broker::relay({_id, data.value, data.alters}, sender);
    }
    return Broker::relay(data, sender);
  }

  Id id() const;
  Id bookId() const;
  bool append(Amount value);
  bool append(const Data& entry);
  bool replace(Amount value, const Clock& clock);
  bool erase(Clock time);
  bool contains(const Clock& clock) const;

private:
  const Id _id;
  static std::unique_ptr<Random> _random;
  const Id _bookId;
  Clock _version;
};

using JournalPtr = std::shared_ptr<JournalBase>;
}

#endif
