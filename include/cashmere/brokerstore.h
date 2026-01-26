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
#ifndef CASHMERE_STORE_H
#define CASHMERE_STORE_H

#include "cashmere/cashmere.h"

namespace Cashmere
{

class CASHMERE_EXPORT BrokerStoreBase : public std::enable_shared_from_this<BrokerStoreBase>
{
public:
  virtual ~BrokerStoreBase();
  virtual BrokerBasePtr getOrCreate(const std::string& url) = 0;
  virtual bool insert(const std::string& url, BrokerBasePtr broker) = 0;
  virtual std::size_t size() const = 0;
};

}

#endif
