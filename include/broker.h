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
#ifndef CASHMERE_BROKER_H
#define CASHMERE_BROKER_H

#include "entry.h"
#include <memory>

namespace Cashmere
{

class Broker;
using BrokerPtr = std::shared_ptr<Broker>;
using BrokerWeakPtr = std::weak_ptr<Broker>;

struct ConnectionInfo
{
  int64_t distance;
  Clock version;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

class Broker : public std::enable_shared_from_this<Broker>
{
  struct Impl;

public:
  Broker();

  virtual ~Broker();

  virtual Id id() const;

  Clock insert(const EntryList& entries, Port sender = 0);
  virtual Clock insert(const Entry& data, Port sender = 0);

  virtual EntryList entries(const Clock& from = {}) const;
  EntryList entries(const Clock& from, Port ignore) const;

  virtual IdConnectionInfoMap provides(Port to = 0) const;
  IdClockMap versions() const;

  Clock clock() const;

  bool attach(BrokerPtr other);
  bool detach(Port port);

  BrokerPtr ptr();

private:
  Impl* impl();
  std::unique_ptr<Impl> b;
};

}

#endif
