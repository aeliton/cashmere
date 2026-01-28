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
#ifndef CASHEMERE_BROKER_STORE_IMPL_H
#define CASHEMERE_BROKER_STORE_IMPL_H

#include "cashmere/brokerstore.h"
#include <functional>
#include <filesystem>
#include <dlfcn.h>

namespace fs = std::filesystem;

namespace Cashmere
{

using BrokerCreator = std::function<BrokerBase*(const std::string&)>;
using SchemaFunctorMap = std::map<std::string, BrokerCreator>;

struct BrokerStore::Impl {
  static SchemaFunctorMap LoadPlugins(const std::string& path)
  {
    SchemaFunctorMap result;
    for (auto& entry : fs::directory_iterator(path)) {
      void* handle = dlopen(entry.path().c_str(), RTLD_LAZY);
      if (!handle) {
        continue;
      }
      dlerror();
      auto func = dlsym(handle, "create");
      char* error = dlerror();
      if (error != nullptr) {
        dlclose(handle);
        continue;
      }
      auto filename = entry.path().filename().string();
      auto schema = filename.substr(3, filename.size() - 6);
      result[schema] = reinterpret_cast<BrokerBase* (*)(const std::string&)>(func);
    }
    return result;
  }
  std::unordered_map<std::string, BrokerBasePtr> store;
  SchemaFunctorMap builders;
};

}
#endif
