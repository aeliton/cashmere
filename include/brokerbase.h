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
#ifndef CASHMERE_BROKER_INTERFACE_H
#define CASHMERE_BROKER_INTERFACE_H

#include <map>
#include <memory>

#include "entry.h"

namespace Cashmere
{

struct ConnectionInfo
{
  int64_t distance;
  Clock version;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

class BrokerBase;
using BrokerBasePtr = std::shared_ptr<BrokerBase>;
using BrokerBaseWeakPtr = std::weak_ptr<BrokerBase>;

class Connection
{
public:
  Connection();
  Connection(
    BrokerBasePtr broker, Port port, Clock version, IdConnectionInfoMap provides
  );
  Clock insert(const Entry& data);
  Clock insert(const EntryList& data);
  EntryList entries(Clock clock = {}) const;
  bool reconnect(Connection conn) const;
  void setProvides(Id id, Clock version);
  void updateProvides();
  void disconnect();
  void setVersion(Clock clock);
  bool provides(Id id) const;

  BrokerBasePtr broker() const;
  Port port() const;
  Clock version() const;
  IdConnectionInfoMap provides() const;

  bool operator==(const Connection& other) const;

private:
  BrokerBaseWeakPtr _broker;
  Port _port;
  Clock _version;
  IdConnectionInfoMap _provides;
};

class BrokerBase
{
public:
  virtual ~BrokerBase();

  virtual Id id() const = 0;
  virtual Clock insert(const Entry& data, Port sender = 0) = 0;
  virtual EntryList entries(const Clock& from = {}) const = 0;
  virtual EntryList entries(const Clock& from, Port ignore) const = 0;
  virtual IdConnectionInfoMap provides(Port to = 0) const = 0;
  virtual IdClockMap versions() const = 0;
  virtual Clock clock() const = 0;
  virtual Port connect(BrokerBasePtr other) = 0;
  virtual Port disconnect(Port port) = 0;
  virtual BrokerBasePtr ptr() = 0;
  virtual void setClock(const Clock& clock) = 0;
  virtual Connection connect(Connection conn) = 0;
  virtual std::set<Port> connectedPorts() const = 0;
  virtual bool update(const Connection& conn, Port sender) = 0;

  Clock insert(const EntryList& entries, Port sender = 0);
};

inline std::ostream& operator<<(std::ostream& os, const ConnectionInfo& info)
{
  return os << "ConnectionInfo{ .distance = " << info.distance
            << ", .version = " << info.version << "}";
}

}
#endif
