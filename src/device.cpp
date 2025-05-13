#include "device.h"
#include <cassert>

namespace Cashmere
{

Random Device::_random{};

Device::Device()
  : Device(_random.next())
{
}

Device::Device(uint64_t poolId)
  : _poolId(poolId)
  , _deviceId(_random.next())
{
  _clock.insert({_deviceId, 0});
}

const uint64_t Device::id() const
{
  return _deviceId;
}

const uint64_t Device::poolId() const
{
  return _poolId;
}

const std::map<uint64_t, uint64_t>& Device::clock() const
{
  return _clock;
}

bool Device::add(uint64_t value)
{
  _clock[_deviceId]++;
  _transactions.insert({_clock, Transaction{value}});
  return true;
}

std::map<std::map<uint64_t, uint64_t>, Transaction> Device::transactions() const
{
  return _transactions;
}

}
