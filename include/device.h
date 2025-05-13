#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

#include "random.h"

namespace Cashmere
{

struct Transaction
{
  uint64_t value;
};

class Device
{
public:
  Device();
  explicit Device(uint64_t poolId);

  const uint64_t id() const;
  const uint64_t poolId() const;
  const std::map<uint64_t, uint64_t>& clock() const;

  bool add(uint64_t value);

  std::map<std::map<uint64_t, uint64_t>, Transaction> transactions() const;

private:
  static Random _random;
  const uint64_t _poolId;
  const uint64_t _deviceId;
  std::map<uint64_t, uint64_t> _clock;
  std::map<std::map<uint64_t, uint64_t>, Transaction> _transactions;
};

}

#endif
