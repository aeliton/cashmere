#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

#include "random.h"

namespace Cashmere
{

using Amount = int64_t;
using Time = uint64_t;

struct Transaction;

class Ledger
{
public:
  using Id = uint64_t;
  using Clock = std::map<Id, Time>;

  enum class Operation
  {
    Insert,
    Replace,
    Delete
  };

  struct Transaction
  {
    struct Id
    {
      Ledger::Id ledgerId;
      Time time;
    };

    Operation operation;
    Amount value;
    Id alters;
  };

  using LedgerTransactions = std::map<Id, Transaction>;
  using BookTransactions = std::map<Id, LedgerTransactions>;

  Ledger();
  explicit Ledger(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(const Transaction& value);
  bool append(Id ledgerId, Amount value);
  bool append(Id ledgerId, const Transaction& value);
  bool insert(Id ledgerId, Time time, Amount value);
  bool insert(Id ledgerId, Time time, const Transaction& value);

  const Transaction& query(Time time) const;
  const Transaction& query(Id ledgerId, Time time) const;

  const BookTransactions& book() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  BookTransactions _book;
};

}

#endif
