#include "device.h"

namespace Cashmere
{

Device::Device()
  : Device(0)
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

const std::vector<uint64_t>& Device::clock() const
{
  return _clock;
}

}
