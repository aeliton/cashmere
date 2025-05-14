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

class Ledger
{
public:
  Ledger();
  explicit Ledger(uint64_t poolId);

  const uint64_t id() const;
  const uint64_t bookId() const;
  const std::map<uint64_t, uint64_t>& clock() const;

  bool add(uint64_t value);

  std::map<std::map<uint64_t, uint64_t>, Transaction> transactions() const;

private:
  static Random _random;
  const uint64_t _bookId;
  const uint64_t _ledgerId;
  std::map<uint64_t, uint64_t> _clock;
  std::map<std::map<uint64_t, uint64_t>, Transaction> _transactions;
};

}

#endif
