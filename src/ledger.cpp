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
  _clock.insert({_ledgerId, 0});
}

const Id Ledger::id() const
{
  return _ledgerId;
}

const Id Ledger::bookId() const
{
  return _bookId;
}

const Clock& Ledger::clock() const
{
  return _clock;
}

bool Ledger::add(Amount value)
{
  return add(_ledgerId, _clock[_ledgerId] + 1, value);
}

bool Ledger::add(Id ledgerId, Time time, Amount value)
{
  auto clock = _clock;
  clock[ledgerId] = time;
  _transactions.insert({clock, Transaction{value}});
  return true;
}

std::map<Clock, Transaction> Ledger::transactions() const
{
  return _transactions;
}

}
