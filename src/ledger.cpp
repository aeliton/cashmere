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

const Id Ledger::id() const
{
  return _ledgerId;
}

const Id Ledger::bookId() const
{
  return _bookId;
}

Clock Ledger::clock() const
{
  Clock clock{{_ledgerId, 0UL}};
  for (auto& [ledgerId, transactions] : _transactions) {
    clock[ledgerId] = transactions.empty() ? 0 : transactions.rbegin()->first;
  }
  return clock;
}

bool Ledger::add(Amount value)
{
  return add(_ledgerId, value);
}

bool Ledger::add(Id ledgerId, Amount value)
{
  Time time =
      _transactions[ledgerId].empty() ? 0UL : _transactions.rbegin()->first;
  return add(ledgerId, time + 1, value);
}

bool Ledger::add(Id ledgerId, Time time, Amount value)
{
  _transactions[ledgerId][time] = value;
  return true;
}

Amount Ledger::query(Time time) const
{
  return _transactions.at(_ledgerId).at(time);
}

Amount Ledger::query(Id ledgerId, Time time) const
{
  return _transactions.at(ledgerId).at(time);
}

std::map<Id, std::map<Time, Amount>> Ledger::transactions() const
{
  return _transactions;
}

}
