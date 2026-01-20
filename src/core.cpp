#include "core.h"

namespace Cashmere {

BrokerStorePtr BrokerStore::_instance = nullptr;

BrokerStore::BrokerStore() = default;

BrokerBasePtr BrokerStore::get([[maybe_unused]] Id id) {
  return nullptr;
}

bool BrokerStore::load([[maybe_unused]] const std::string& path) {
  return {};
}

BrokerStorePtr BrokerStore::instance() {
  if (!_instance) {
    _instance = std::shared_ptr<BrokerStore>(new BrokerStore());
  }
  return _instance;
}

}
