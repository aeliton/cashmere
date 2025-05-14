#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

#include "random.h"

namespace Cashmere
{

using Id = uint64_t;
using Clock = std::map<Id, uint64_t>;

struct Transaction
{
  uint64_t value;
};

class Ledger
{
public:
  Ledger();
  explicit Ledger(uint64_t poolId);

  const Id id() const;
  const Id bookId() const;
  const Clock& clock() const;

  bool add(uint64_t value);

  std::map<std::map<uint64_t, uint64_t>, Transaction> transactions() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _ledgerId;
  Clock _clock;
  std::map<Clock, Transaction> _transactions;
};

}

#endif
