#include "cashmere/utils/url.h"
#include "cashmere/utils/file.h"
#include "cashmere/brokerwrapper.h"
#include <dlfcn.h>

namespace fs = std::filesystem;

namespace Cashmere {

std::string WrappersDirectory()
{
  return InstallDirectory() / "lib/cashmere/wrappers";
}

using BrokerCreator = std::function<WrapperBase*(const std::string&)>;
using SchemaFunctorMap = std::map<std::string, BrokerCreator>;

SchemaFunctorMap LoadWrapperPlugins(const std::string& path)
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
    result[schema] = reinterpret_cast<WrapperBase* (*)(const std::string&)>(func);
  }
  return result;
}

struct WrapperStore::Impl {
    std::unordered_map<std::string, WrapperBasePtr> store;
    SchemaFunctorMap builders;
};

WrapperBasePtr WrapperStore::getOrCreate(const std::string& url)
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
  return _impl->store[url] = std::shared_ptr<WrapperBase>(builderIt->second(url));
}

std::size_t WrapperStore::size() const
{
  return _impl->store.size();
}

WrapperStoreBasePtr WrapperStore::create()
{
  return std::make_shared<WrapperStore>(Private());
}

WrapperStore::WrapperStore(Private)
  : WrapperStoreBase()
  , _impl(std::make_unique<Impl>())
{
  _impl->builders = LoadWrapperPlugins(WrappersDirectory());
}
}
