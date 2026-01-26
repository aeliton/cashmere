#include "broker.h"
#include "brokergrpcstub.h"
#include "journal.h"
#include "journalfile.h"
#include "utils/urlutils.h"
#include "brokerbaseimpl.h"
#include "grpcrunner.h"

namespace Cashmere {

BrokerStore::BrokerStore(Private)
{
  _builders["hub"] = &Broker::create;
  _builders["file"] = &JournalFile::create;
  _builders["grpc"] = &BrokerGrpcStub::create;
  _builders["cache"] = &Journal::create;
}

BrokerStoreBasePtr BrokerStore::create()
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
  broker->impl()->setStore(shared_from_this());
  _store[url] = broker;
  return broker;
}

std::size_t BrokerStore::size() const
{
  return _store.size();
}

bool BrokerStore::insert(const std::string& url, BrokerBasePtr broker)
{
  broker->impl()->setStore(shared_from_this());
  _store[url] = broker;
  return {};
}

WrapperBasePtr WrapperStore::getOrCreate(const std::string& url)
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
  return _store[url] = builderIt->second(url);
}

std::size_t WrapperStore::size() const
{
  return _store.size();
}
WrapperStore::WrapperStore()
  : WrapperStoreBase()
  , _store()
  , _builders()
{
  _builders["grpc"] = &GrpcRunner::create;
}
}
