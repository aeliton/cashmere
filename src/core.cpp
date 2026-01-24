#include "core.h"
#include "cashmere/broker.h"
#include "cashmere/brokergrpc.h"
#include "cashmere/journal.h"
#include "cashmere/journalfile.h"
#include "utils/urlutils.h"

namespace Cashmere {

BrokerStore::BrokerStore()
{
  _builders["hub"] = &Broker::create;
  _builders["file"] = &JournalFile::create;
  _builders["grpc"] = &BrokerGrpc::create;
  _builders["cache"] = &Journal::create;
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
