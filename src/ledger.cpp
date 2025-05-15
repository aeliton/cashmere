#include "ledger.h"
#include <cassert>

namespace Cashmere
{

Random Ledger::_random{};

Ledger::Ledger()
  : Ledger(_random.next())
{
}

Ledger::Ledger(Id poolId)
  : _bookId(poolId)
  , _ledgerId(_random.next())
{
}

const Ledger::Id Ledger::id() const
{
  return _ledgerId;
}

const Ledger::Id Ledger::bookId() const
{
  return _bookId;
}

Ledger::Clock Ledger::clock() const
{
  Clock clock{{_ledgerId, 0UL}};
  for (auto& [ledgerId, transactions] : _transactions) {
    clock[ledgerId] = transactions.empty() ? 0 : transactions.rbegin()->first;
  }
  return clock;
}

bool Ledger::append(Amount value)
{
  return append(_ledgerId, {Operation::Insert, value, {}});
}

bool Ledger::append(const Transaction& value)
{
  return append(_ledgerId, value);
}

bool Ledger::append(Id ledgerId, Amount value)
{
  return append(ledgerId, {Operation::Insert, value, {}});
}

bool Ledger::append(Id ledgerId, const Transaction& value)
{
  Time time =
      _transactions[ledgerId].empty() ? 0UL : _transactions.rbegin()->first;
  return insert(ledgerId, time + 1, value);
}

bool Ledger::insert(Id ledgerId, Time time, Amount value)
{
  _transactions[ledgerId][time] = {Operation::Insert, value, {}};
  return true;
}

bool Ledger::insert(Id ledgerId, Time time, const Transaction& value)
{
  _transactions[ledgerId][time] = value;
  return true;
}

const Ledger::Transaction& Ledger::query(Time time) const
{
  return _transactions.at(_ledgerId).at(time);
}

const Ledger::Transaction& Ledger::query(Id ledgerId, Time time) const
{
  return _transactions.at(ledgerId).at(time);
}

const Ledger::BookTransactions& Ledger::transactions() const
{
  return _transactions;
}

}
