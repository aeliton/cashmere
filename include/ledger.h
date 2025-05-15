#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

#include "random.h"

namespace Cashmere
{

using Id = uint64_t;
using Amount = int64_t;
using Clock = std::map<Id, uint64_t>;

struct Transaction
{
  Amount value;
};

class Ledger
{
public:
  Ledger();
  explicit Ledger(uint64_t poolId);

  const Id id() const;
  const Id bookId() const;
  const Clock& clock() const;

  bool add(Amount value);
  bool add(Id ledgerId, Amount value);

  std::map<Clock, Transaction> transactions() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _ledgerId;
  Clock _clock;
  std::map<Clock, Transaction> _transactions;
};

}

#endif
