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
#ifndef CASHMERE_CORE_H
#define CASHMERE_CORE_H

#include "cashmere/brokerstore.h"
#include "utils/urlutils.h"

#include <functional>
#include <unordered_map>

namespace Cashmere {
  
class BrokerStore;
using BrokerStorePtr = std::shared_ptr<BrokerStore>;

class BrokerStore : public BrokerStoreBase {
  struct Private{ explicit Private() = default; };
  public:
    BrokerStore(Private);
    static BrokerStorePtr create();
    BrokerBasePtr build(const std::string& url) override;
    bool insert(const std::string& url, BrokerBasePtr broker) override;
    BrokerBasePtr get(const std::string& url) override;
    std::size_t size() const override;

  private:
    std::unordered_map<std::string, BrokerBasePtr> _store;
    std::unordered_map<std::string, std::function<BrokerBasePtr(const std::string&)>> _builders;
};

}

#endif
