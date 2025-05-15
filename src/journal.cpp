#include "journal.h"
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
  for (auto& [journalId, transactions] : _book) {
    clock[journalId] = transactions.empty() ? 0 : transactions.rbegin()->first;
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

bool Journal::append(Id journalId, Amount value)
{
  return append(journalId, {Operation::Insert, value, {}});
}

bool Journal::append(Id journalId, const Entry& value)
{
  Time time = _book[journalId].empty() ? 0UL : _book[journalId].rbegin()->first;
  return insert(journalId, time + 1, value);
}

bool Journal::insert(Id journalId, Time time, Amount value)
{
  _book[journalId][time] = {Operation::Insert, value, {}};
  return true;
}

bool Journal::insert(Id journalId, Time time, const Entry& value)
{
  _book[journalId][time] = value;
  return true;
}

bool Journal::replace(Id journalId, Time time, Amount value)
{
  return append({Operation::Replace, value, {journalId, time}});
}

bool Journal::erase(Id journalId, Time time)
{
  return append({Operation::Delete, 0, {journalId, time}});
}

const Journal::Entry& Journal::query(Time time) const
{
  return _book.at(_id).at(time);
}

const Journal::Entry& Journal::query(Id journalId, Time time) const
{
  return _book.at(journalId).at(time);
}

const Journal::JournalBook& Journal::journals() const
{
  return _book;
}

}
