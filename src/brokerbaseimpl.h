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
#ifndef CASHMERE_BROKER_BASE_IMPL_H
#define CASHMERE_BROKER_BASE_IMPL_H

#include "cashmere/brokerbase.h"
#include "cashmere/brokerstore.h"
#include "utils/random.h"
#include "utils/urlutils.h"

namespace Cashmere
{

struct BrokerBase::Impl {
  Impl(const std::string& u);
  void setStore(BrokerStoreBasePtr value);
  BrokerStoreBasePtr store();

  Url url;
  Id id;
  std::string hostname;
  uint16_t port;
  BrokerStoreBaseWeakPtr storePtr;
  static std::unique_ptr<Random> random;
};

}

#endif
