#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

#include "random.h"

namespace Cashmere
{

using Id = uint64_t;
using Amount = int64_t;
using Time = uint64_t;
using Clock = std::map<Id, Time>;

class Ledger
{
public:
  Ledger();
  explicit Ledger(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool add(Amount value);
  bool add(Id ledgerId, Amount value);
  bool add(Id ledgerId, Time time, Amount value);

  std::map<Id, std::map<Time, Amount>> transactions() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _ledgerId;
  std::map<Id, std::map<Time, Amount>> _transactions;
};

}

#endif
