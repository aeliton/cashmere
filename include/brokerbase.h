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
#include <proto/cashmere.grpc.pb.h>

namespace Cashmere
{

struct ConnectionInfo
{
  int16_t distance;
  Clock version;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
  friend std::ostream& operator<<(std::ostream& os, const ConnectionInfo& info);
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

class BrokerBase;
using BrokerBasePtr = std::shared_ptr<BrokerBase>;
using BrokerBaseWeakPtr = std::weak_ptr<BrokerBase>;

struct ConnectionData
{
  Port port;
  Clock version;
  IdConnectionInfoMap sources;
  bool operator==(const ConnectionData& other) const;
  friend std::ostream& operator<<(std::ostream& os, const ConnectionData& Data);
};

class BrokerStub;
using BrokerStubPtr = std::shared_ptr<BrokerStub>;

class BrokerStub
{
public:
  enum class Type
  {
    Invalid,
    Memory,
    Grpc
  };

  virtual ~BrokerStub();

  explicit BrokerStub();
  explicit BrokerStub(BrokerBasePtr broker, Type type = Type::Memory);
  explicit BrokerStub(const std::string& url);

  virtual BrokerBasePtr broker() const;
  Type type() const;

  void reset();

private:
  Type _type;
  BrokerBaseWeakPtr _broker;
};

class Connection
{
public:
  enum class Origin
  {
    Cache,
    Remote
  };
  explicit Connection();
  Connection(BrokerStubPtr stub, ConnectionData data);
  Connection(
    BrokerStubPtr stub, Port port, Clock version, IdConnectionInfoMap provides
  );
  Connection(
    const std::string& url, Port port, Clock version,
    IdConnectionInfoMap provides
  );

  virtual ~Connection();

  BrokerBasePtr broker() const;
  void disconnect();

  Clock insert(const Entry& data) const;
  Clock insert(const EntryList& data) const;
  EntryList entries(const Clock& clock = {}) const;
  bool active() const;
  bool refresh(const ConnectionData& conn) const;

  Port port() const;
  Clock& version(Origin origin = Origin::Cache) const;
  IdConnectionInfoMap& provides(Origin origin = Origin::Cache) const;

  bool operator==(const Connection& other) const;
  friend std::ostream& operator<<(std::ostream& os, const Connection& data);

protected:
  friend class Broker;

  void update(const ConnectionData& data);

  BrokerStubPtr _broker;
  mutable ConnectionData _cache;
};

class BrokerBase
{
public:
  virtual ~BrokerBase();

  virtual Id id() const = 0;
  virtual Clock clock() const = 0;
  virtual IdClockMap versions() const = 0;
  virtual void setClock(const Clock& clock) = 0;
  virtual IdConnectionInfoMap provides(Port sender = 0) const = 0;
  virtual Clock insert(const Entry& data, Port sender = 0) = 0;
  virtual EntryList query(const Clock& from = {}, Port sender = 0) const = 0;

  virtual ConnectionData connect(Connection conn) = 0;
  virtual bool refresh(const ConnectionData& conn, Port sender) = 0;
  virtual Port disconnect(Port port) = 0;
  virtual std::set<Port> connectedPorts() const = 0;

  virtual BrokerBasePtr ptr() = 0;

  Clock insert(const EntryList& entries, Port sender = 0);
};

std::ostream& operator<<(std::ostream& os, const IdConnectionInfoMap& data);

}
#endif
