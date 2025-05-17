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
  , _clock({{_id, 0}})
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
  return _clock;
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
  _clock[journalId]++;
  _book[_clock] = value;
  return true;
}

bool Journal::replace(Id journalId, const Clock& clock, Amount value)
{
  return append({Operation::Replace, value, clock});
}

bool Journal::erase(Id journalId, const Clock& time)
{
  return append(journalId, {Operation::Delete, 0, time});
}

Journal::Entry Journal::query(const Clock& time) const
{
  if (_book.find(time) == _book.end()) {
    return {Journal::Operation::Invalid, 0, {}};
  }

  return _book.at(time);
}

const Journal::JournalEntries& Journal::journals() const
{
  return _book;
}

}
