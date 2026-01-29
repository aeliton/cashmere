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

#include <memory>

#include "cashmere/cashmere.h"
#include "cashmere/connection.h"
#include "cashmere/entry.h"

namespace Cashmere
{

class BrokerStoreBase;
using BrokerStoreBasePtr = std::shared_ptr<BrokerStoreBase>;

class CASHMERE_EXPORT BrokerBase : public std::enable_shared_from_this<BrokerBase>
{
  struct Impl;
  std::unique_ptr<Impl> _impl;
public:
  BrokerBase(const std::string& url = {});
  virtual ~BrokerBase();

  virtual Id id() const;
  virtual std::string url() const;
  virtual std::string schema() const = 0;
  virtual Connection connect(Connection conn) = 0;
  virtual bool refresh(const Connection& conn, Source sender) = 0;
  virtual Clock insert(const Entry& data, Source sender = 0) = 0;
  virtual Clock insert(const EntryList& entries, Source sender = 0);

  virtual EntryList query(const Clock& from = {}, Source sender = 0) const = 0;
  virtual Clock clock() const = 0;
  virtual IdClockMap versions() const = 0;
  virtual SourcesMap sources(Source sender = 0) const = 0;
  virtual Clock relay(const Data& entry, Source sender) = 0;
  virtual std::set<Source> connectedPorts() const = 0;
  virtual Source disconnect(Source source) = 0;
  virtual Data entry(Clock) const;

  virtual EntryList entries() const;

  virtual Connection stub();
  virtual Connection connect(const std::string& url);
  virtual bool save(const Entry&);
  virtual bool append(Amount value);
  virtual bool append(const Data& entry);
  virtual bool replace(Amount value, const Clock& clock);
  virtual bool erase(Clock time);
  virtual bool contains(const Clock& clock) const;

  virtual std::string location() const;
  virtual uint16_t port() const;
  virtual std::string hostname() const;

  virtual BrokerBasePtr ptr();

  virtual BrokerStoreBasePtr store();

  Impl* impl();
};

CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdConnectionInfoMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const IdClockMap& data);
CASHMERE_EXPORT std::ostream&
operator<<(std::ostream& os, const SourcesMap& data);

}
#endif
