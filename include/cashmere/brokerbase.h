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
  Clock clock;
  bool operator==(const ConnectionInfo& other) const;
  bool operator<(const ConnectionInfo& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const ConnectionInfo& info);
};

using IdConnectionInfoMap = std::map<Id, ConnectionInfo>;

using SourcesMap = std::map<Source, IdConnectionInfoMap>;

class BrokerBase;
using BrokerBasePtr = std::shared_ptr<BrokerBase>;
using BrokerBaseWeakPtr = std::weak_ptr<BrokerBase>;

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class CASHMERE_EXPORT Connection
{
public:
  enum class Origin
  {
    Cache,
    Remote
  };
  enum class Type
  {
    Invalid,
    Memory,
    Grpc
  };

  virtual ~Connection();

  explicit Connection();
  explicit Connection(BrokerBasePtr broker, Type type = Type::Memory);
  explicit Connection(
    BrokerBasePtr broker, Source source, const Clock& version,
    const IdConnectionInfoMap& sources
  );
  explicit Connection(const std::string& url);

  Connection& connect(Connection conn);
  bool refresh(const Connection& conn) const;
  Clock insert(const Entry& data) const;
  Clock insert(const EntryList& data) const;

  EntryList query(const Clock& clock = {}) const;
  Clock& clock(Origin origin = Origin::Cache) const;
  Clock relay(const Data& entry) const;

  IdConnectionInfoMap& provides(Origin origin = Origin::Cache) const;

  void disconnect();
  bool valid() const;
  Source& source() const;

  std::string url() const;
  Type type() const;
  void reset();
  std::string str() const;

  bool operator==(const Connection& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const Connection& data);

private:
  virtual BrokerBasePtr broker() const;
  std::string _url;
  Type _type;
  mutable Source _source;
  mutable Clock _version;
  mutable IdConnectionInfoMap _sources;
  BrokerBaseWeakPtr _memoryStub;
  BrokerBasePtr _grpcStub;
};

class CASHMERE_EXPORT BrokerBase
{
public:
  virtual ~BrokerBase();

  virtual Connection connect(Connection conn) = 0;
  virtual bool refresh(const Connection& conn, Source sender) = 0;
  virtual Clock insert(const Entry& data, Source sender = 0) = 0;
  virtual Clock insert(const EntryList& entries, Source sender = 0);

  virtual EntryList query(const Clock& from = {}, Source sender = 0) const = 0;
  virtual Clock clock() const = 0;
  virtual IdClockMap versions() const = 0;
  virtual SourcesMap sources(Source sender = 0) const = 0;
  virtual Clock relay(const Data& entry, Source sender) = 0;
  virtual Connection stub() = 0;
};

CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdConnectionInfoMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdClockMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const SourcesMap& data);

}
#endif
