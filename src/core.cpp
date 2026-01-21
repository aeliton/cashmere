#include "core.h"
#include "cashmere/broker.h"
#include "cashmere/journalfile.h"
#include "cashmere/brokergrpc.h"
#include "utils/urlutils.h"

namespace Cashmere {

BrokerStorePtr BrokerStore::_instance = nullptr;

BrokerStore::BrokerStore()
{
  _builders["hub"] = &Broker::create;
  _builders["file"] = &JournalFile::create;
  _builders["grpc"] = &BrokerGrpc::create;
}

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


BrokerBasePtr BrokerStore::build(const std::string& url) const
{
  const Url parsed = ParseUrl(url);
  const auto it = _builders.find(parsed.schema);
  if (it == _builders.end()) {
    return nullptr;
  }
  return it->second(parsed);
}

}
