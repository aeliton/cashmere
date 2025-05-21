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
  , _clock({})
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

Journal::Clock Journal::merge(const Clock& a, const Clock& b)
{
  auto out = a;
  for (auto& [id, count] : b) {
    out[id] = std::max(a.find(id) != a.end() ? a.at(id) : 0, count);
  }
  return out;
}

bool Journal::smaller(const Clock& a, const Clock& b)
{
  const auto top = merge(a, b);
  return top == b;
}

bool Journal::append(Id journalId, Amount value)
{
  return append({journalId, value, {}});
}

bool Journal::append(Amount value)
{
  return append({_id, value, {}});
}

bool Journal::append(Entry value)
{
  _clock[value.journalId]++;
  return insert(_clock, value);
}

bool Journal::insert(Clock clock, Entry value)
{
  std::erase_if(
      value.alters, [](const auto& item) { return item.second == 0; });
  std::erase_if(clock, [](const auto& item) { return item.second == 0; });
  _entries[clock] = value;
  _clock = merge(clock, _clock);
  return true;
}

bool Journal::replace(Amount value, const Clock& clock)
{
  return append({_id, value, clock});
}

bool Journal::replace(Id journalId, Amount value, const Clock& clock)
{
  return append({journalId, value, clock});
}

bool Journal::erase(Clock time)
{
  return append({_id, 0, time});
}

bool Journal::erase(Id journalId, Clock time)
{
  return append({journalId, 0, time});
}

Journal::Entry Journal::query(Clock time) const
{
  std::erase_if(time, [](const auto& item) { return item.second == 0; });

  if (_entries.find(time) == _entries.end()) {
    return {0, 0, {{0UL, 0}}};
  }

  return _entries.at(time);
}

const Journal::JournalEntries& Journal::entries() const
{
  return _entries;
}

}
