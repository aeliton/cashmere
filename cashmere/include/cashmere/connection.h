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
#ifndef CASHMERE_CONNECTION_H
#define CASHMERE_CONNECTION_H

#include <memory>

#include "cashmere/cashmere.h"
#include "cashmere/connectioninfo.h"
#include "cashmere/entry.h"

namespace Cashmere
{

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class BrokerBase;
using BrokerBasePtr = std::shared_ptr<BrokerBase>;
using BrokerBaseWeakPtr = std::weak_ptr<BrokerBase>;

class BrokerStoreBase;
using BrokerStoreBasePtr = std::shared_ptr<BrokerStoreBase>;

class CASHMERE_EXPORT Connection
{
public:
  enum class Origin
  {
    Cache,
    Remote
  };

  virtual ~Connection();

  explicit Connection();
  explicit Connection(
    BrokerBasePtr broker, Source source = {}, const Clock& version = {},
    const IdConnectionInfoMap& sources = {}
  );

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
  void reset();
  std::string str() const;

  bool operator==(const Connection& other) const;
  CASHMERE_EXPORT friend std::ostream&
  operator<<(std::ostream& os, const Connection& data);

private:
  virtual BrokerBasePtr broker() const;
  mutable Source _source;
  mutable Clock _version;
  mutable IdConnectionInfoMap _sources;
  BrokerBaseWeakPtr _broker;
};

}
#endif
