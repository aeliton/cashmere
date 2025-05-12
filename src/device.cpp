#include "device.h"
#include <cassert>

namespace Cashmere
{
using std::vector;

Random Device::_random{};

Device::Device()
  : Device(_random.next())
{
}

Device::Device(uint64_t poolId)
  : _poolId(poolId)
  , _deviceId(0)
{
  _clock.push_back(0);
}

const uint8_t Device::id() const
{
  return _deviceId;
}

const uint64_t Device::poolId() const
{
  return _poolId;
}

const std::vector<uint64_t>& Device::clock() const
{
  return _clock;
}

bool Device::add(uint64_t value)
{
  size_t previousSize = _transactions.size();
  _clock[_deviceId]++;
  _transactions.insert({_clock, Transaction{value}});
  assert(_transactions.size() == previousSize + 1);
  return true;
}

std::map<std::vector<uint64_t>, Transaction> Device::transactions() const
{
  return _transactions;
}

}
