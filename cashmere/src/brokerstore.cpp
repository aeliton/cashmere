#include "cashmere/utils/url.h"
#include "cashmere/utils/file.h"
#include "brokerbaseimpl.h"
#include "storeimpl.h"
#include "cashmere/brokerwrapper.h"

namespace Cashmere {

std::string PluginsDirectory()
{
  return InstallDirectory() / "lib/cashmere/plugins";
}

BrokerStore::BrokerStore(Private)
  : _impl(std::make_unique<Impl>())
{
  _impl->builders = Impl::LoadPlugins(PluginsDirectory());
}

BrokerStoreBasePtr BrokerStore::create()
{
  return std::make_shared<BrokerStore>(Private());
}

BrokerBasePtr BrokerStore::getOrCreate(const std::string& url)
{
  auto storeIt = _impl->store.find(url);
  if (storeIt != _impl->store.end()) {
    return storeIt->second;
  }
  const Url parsed = ParseUrl(url);
  const auto builderIt = _impl->builders.find(parsed.schema);
  if (builderIt == _impl->builders.end()) {
    return nullptr;
  }
  auto broker = std::shared_ptr<BrokerBase>(builderIt->second(url));
  broker->impl()->setStore(shared_from_this());
  _impl->store[url] = broker;
  return broker;
}

std::size_t BrokerStore::size() const
{
  return _impl->store.size();
}

bool BrokerStore::insert(const std::string& url, BrokerBasePtr broker)
{
  broker->impl()->setStore(shared_from_this());
  _impl->store[url] = broker;
  return {};
}

}
