// Cashmere - a distributed conflict-free replicated database.
// Copyright (C) 2026 Aeliton G. Silva
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
#ifndef CASHMERE_BROKER_WRAPPER_BASE_H
#define CASHMERE_BROKER_WRAPPER_BASE_H

#include "cashmere/brokerbase.h"

#include <thread>
#include <functional>
#include <unordered_map>
#include <string>

namespace Cashmere
{

class WrapperBase;
using WrapperBasePtr = std::shared_ptr<WrapperBase>;

class CASHMERE_EXPORT WrapperBase
{
public:
  WrapperBase(const std::string& url);
  virtual ~WrapperBase();

  virtual std::thread start(BrokerBasePtr broker) = 0;
  virtual void stop() = 0;

  std::string url() const;

private:
  std::string _url;
  BrokerBaseWeakPtr _broker;
};

class WrapperStoreBase;
using WrapperStoreBasePtr = std::shared_ptr<WrapperStoreBase>;

class CASHMERE_EXPORT WrapperStoreBase : public std::enable_shared_from_this<WrapperStoreBase>
{
public:
  virtual ~WrapperStoreBase();
  virtual WrapperBasePtr getOrCreate(const std::string& url) = 0;
  virtual std::size_t size() const = 0;
};

class WrapperStore : public WrapperStoreBase
{
  public:
    WrapperStore();
    WrapperBasePtr getOrCreate(const std::string& url) override;

    std::size_t size() const override;

  private:
    std::unordered_map<std::string, WrapperBasePtr> _store;
    std::unordered_map<std::string, std::function<WrapperBasePtr(const std::string&)>> _builders;
};


}

#endif
