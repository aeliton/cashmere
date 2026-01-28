#include "hub.h"

extern "C" CASHMERE_EXPORT Cashmere::BrokerBase* create(const std::string& url)
{
  return new Cashmere::Broker(url);
}
