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
#include "brokerbaseimpl.h"

namespace Cashmere
{

std::unique_ptr<Random> BrokerBase::Impl::random = std::make_unique<Random>();

BrokerBase::Impl::Impl(const std::string& u)
  : url(ParseUrl(u))
{
  try {
    id = std::stoul(url.id, nullptr, 16);
  } catch (std::exception) {
    id = random->next();
  }
  size_t pos = url.hostport.find(':');
  hostname = url.hostport.substr(0, pos);
  try {
    port = std::stoul(url.hostport.substr(pos + 1), nullptr);
  } catch (std::exception) {
    port = 0;
  }
}

void BrokerBase::Impl::setStore(BrokerStoreBasePtr value)
{
  storePtr = value;
}

BrokerStoreBasePtr BrokerBase::Impl::store()
{
  return storePtr.lock();
}

}
