#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <vector>

#include "random.h"

namespace Cashmere
{

class Device
{
public:
  Device();
  explicit Device(uint64_t poolId);

  const uint8_t id() const;
  const uint64_t poolId() const;
  const std::vector<uint64_t>& clock() const;

  bool add(uint64_t value);

private:
  static Random _random;
  const uint64_t _poolId;
  const uint8_t _deviceId;
  std::vector<uint64_t> _clock;
};

}

#endif
