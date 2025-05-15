#include "ledger.h"
#include <cassert>

namespace Cashmere
{

Random Journal::_random{};

Journal::Journal()
  : Journal(_random.next())
{
}

Journal::Journal(Id poolId)
  : _bookId(poolId)
  , _id(_random.next())
{
}

const Journal::Id Journal::id() const
{
  return _id;
}

const Journal::Id Journal::bookId() const
{
  return _bookId;
}

Journal::Clock Journal::clock() const
{
  Clock clock{{_id, 0UL}};
  for (auto& [ledgerId, transactions] : _book) {
    clock[ledgerId] = transactions.empty() ? 0 : transactions.rbegin()->first;
  }
  return clock;
}

bool Journal::append(Amount value)
{
  return append(_id, {Operation::Insert, value, {}});
}

bool Journal::append(const Entry& value)
{
  return append(_id, value);
}

bool Journal::append(Id ledgerId, Amount value)
{
  return append(ledgerId, {Operation::Insert, value, {}});
}

bool Journal::append(Id ledgerId, const Entry& value)
{
  Time time = _book[ledgerId].empty() ? 0UL : _book.rbegin()->first;
  return insert(ledgerId, time + 1, value);
}

bool Journal::insert(Id ledgerId, Time time, Amount value)
{
  _book[ledgerId][time] = {Operation::Insert, value, {}};
  return true;
}

bool Journal::insert(Id ledgerId, Time time, const Entry& value)
{
  _book[ledgerId][time] = value;
  return true;
}

const Journal::Entry& Journal::query(Time time) const
{
  return _book.at(_id).at(time);
}

const Journal::Entry& Journal::query(Id ledgerId, Time time) const
{
  return _book.at(ledgerId).at(time);
}

const Journal::JournalBook& Journal::journals() const
{
  return _book;
}

}
