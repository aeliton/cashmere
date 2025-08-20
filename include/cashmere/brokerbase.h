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

#include "cashmere/entry.h"

namespace Cashmere
{

struct CASHMERE_EXPORT ConnectionInfo
{
  int16_t distance;
  Clock version;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const ConnectionInfo& info);
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

using SourcesMap = std::map<Port, IdConnectionInfoMap>;

class BrokerBase;
using BrokerBasePtr = std::shared_ptr<BrokerBase>;
using BrokerBaseWeakPtr = std::weak_ptr<BrokerBase>;

class BrokerStub;
using BrokerStubPtr = std::shared_ptr<BrokerStub>;

class CASHMERE_EXPORT BrokerStub
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

  std::string url() const;
  Type type() const;

  void reset();
  virtual BrokerBasePtr broker() const;

private:
  std::string _url;
  Type _type;
  BrokerBaseWeakPtr _memoryStub;
  BrokerBasePtr _grpcStub;
};

struct CASHMERE_EXPORT ConnectionData
{
  Port port = 0;
  Clock version = {};
  IdConnectionInfoMap sources = {};
  bool operator==(const ConnectionData& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const ConnectionData& Data);
};

class CASHMERE_EXPORT Connection
{
public:
  enum class Origin
  {
    Cache,
    Remote
  };
  explicit Connection();
  Connection(BrokerStub stub);
  Connection(BrokerStub stub, ConnectionData data);
  Connection(
    BrokerStub stub, Port port, Clock version, IdConnectionInfoMap provides
  );

  virtual ~Connection();

  BrokerBasePtr broker() const;
  void disconnect();

  Clock insert(const Entry& data) const;
  Clock insert(const EntryList& data) const;
  EntryList entries(const Clock& clock = {}) const;
  bool active() const;
  bool refresh(const Connection& conn) const;

  Port& port() const;
  Clock& version(Origin origin = Origin::Cache) const;
  IdConnectionInfoMap& provides(Origin origin = Origin::Cache) const;
  Clock relay(const Data& entry) const;

  bool operator==(const Connection& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const Connection& data);
  void update(const Connection& data);

  BrokerStub& stub();
  ConnectionData cache() const;

  bool valid() const;
  std::string str() const;

protected:
  BrokerStub _broker;
  mutable ConnectionData _cache;
};

class CASHMERE_EXPORT BrokerBase
{
public:
  virtual ~BrokerBase();

  virtual Connection connect(Connection conn) = 0;
  virtual bool refresh(const Connection& conn, Port sender) = 0;
  virtual Clock insert(const Entry& data, Port sender = 0) = 0;
  virtual Clock insert(const EntryList& entries, Port sender = 0);

  virtual EntryList query(const Clock& from = {}, Port sender = 0) const = 0;
  virtual Clock clock() const = 0;
  virtual IdClockMap versions() const = 0;
  virtual SourcesMap sources(Port sender = 0) const = 0;
  virtual Clock relay(const Data& entry, Port sender) = 0;
  virtual BrokerStub stub() = 0;
};

CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdConnectionInfoMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdClockMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const SourcesMap& data);

}
#endif
