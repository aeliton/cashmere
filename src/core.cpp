#include "core.h"
#include "cashmere/broker.h"
#include "brokergrpcstub.h"
#include "cashmere/journal.h"
#include "cashmere/journalfile.h"
#include "utils/urlutils.h"

namespace Cashmere {

BrokerStore::BrokerStore(Private)
{
  _builders["hub"] = &Broker::create;
  _builders["file"] = &JournalFile::create;
  _builders["grpc"] = &BrokerGrpcStub::create;
  _builders["cache"] = &Journal::create;
}

BrokerStorePtr BrokerStore::create()
{
  return std::make_shared<BrokerStore>(Private());
}

BrokerBasePtr BrokerStore::getOrCreate(const std::string& url)
{
  auto storeIt = _store.find(url);
  if (storeIt != _store.end()) {
    return storeIt->second;
  }
  const Url parsed = ParseUrl(url);
  const auto builderIt = _builders.find(parsed.schema);
  if (builderIt == _builders.end()) {
    return nullptr;
  }
  auto broker = builderIt->second(url);
  broker->setStore(shared_from_this());
  _store[url] = broker;
  return broker;
}

std::size_t BrokerStore::size() const
{
  return _store.size();
}

bool BrokerStore::insert(const std::string& url, BrokerBasePtr broker)
{
  broker->setStore(shared_from_this());
  _store[url] = broker;
  return {};
}

}
