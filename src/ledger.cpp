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
  , _id(_random.next())
{
}

const Ledger::Id Ledger::id() const
{
  return _id;
}

const Ledger::Id Ledger::bookId() const
{
  return _bookId;
}

Ledger::Clock Ledger::clock() const
{
  Clock clock{{_id, 0UL}};
  for (auto& [ledgerId, transactions] : _book) {
    clock[ledgerId] = transactions.empty() ? 0 : transactions.rbegin()->first;
  }
  return clock;
}

bool Ledger::append(Amount value)
{
  return append(_id, {Operation::Insert, value, {}});
}

bool Ledger::append(const Transaction& value)
{
  return append(_id, value);
}

bool Ledger::append(Id ledgerId, Amount value)
{
  return append(ledgerId, {Operation::Insert, value, {}});
}

bool Ledger::append(Id ledgerId, const Transaction& value)
{
  Time time = _book[ledgerId].empty() ? 0UL : _book.rbegin()->first;
  return insert(ledgerId, time + 1, value);
}

bool Ledger::insert(Id ledgerId, Time time, Amount value)
{
  _book[ledgerId][time] = {Operation::Insert, value, {}};
  return true;
}

bool Ledger::insert(Id ledgerId, Time time, const Transaction& value)
{
  _book[ledgerId][time] = value;
  return true;
}

const Ledger::Transaction& Ledger::query(Time time) const
{
  return _book.at(_id).at(time);
}

const Ledger::Transaction& Ledger::query(Id ledgerId, Time time) const
{
  return _book.at(ledgerId).at(time);
}

const Ledger::BookTransactions& Ledger::book() const
{
  return _book;
}

}
