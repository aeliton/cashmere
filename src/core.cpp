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

BrokerBasePtr BrokerStore::build(const std::string& url)
{
  const Url parsed = ParseUrl(url);
  const auto it = _builders.find(parsed.schema);
  if (it == _builders.end()) {
    return nullptr;
  }
  auto broker = it->second(url);
  broker->setStore(shared_from_this());
  _store[url] = broker;
  return broker;
}

BrokerBasePtr BrokerStore::get(const std::string& url)
{
  auto it = _store.find(url);
  if (it == _store.end()) {
    return nullptr;
  }
  return it->second;
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
