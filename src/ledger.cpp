#include "ledger.h"
#include <cassert>

namespace Cashmere
{

Random Ledger::_random{};

Ledger::Ledger()
  : Ledger(_random.next())
{
}

Ledger::Ledger(uint64_t poolId)
  : _bookId(poolId)
  , _ledgerId(_random.next())
{
  _clock.insert({_ledgerId, 0});
}

const uint64_t Ledger::id() const
{
  return _ledgerId;
}

const uint64_t Ledger::bookId() const
{
  return _bookId;
}

const std::map<uint64_t, uint64_t>& Ledger::clock() const
{
  return _clock;
}

bool Ledger::add(uint64_t value)
{
  _clock[_ledgerId]++;
  _transactions.insert({_clock, Transaction{value}});
  return true;
}

std::map<std::map<uint64_t, uint64_t>, Transaction> Ledger::transactions() const
{
  return _transactions;
}

}
