#ifndef CASHEMERE_JOURNAL_H
#define CASHEMERE_JOURNAL_H

#include <cstdint>
#include <map>
#include <memory>

#include "random.h"

namespace Cashmere
{

using Amount = int64_t;
using Time = uint64_t;

struct Transaction;

class Journal
{
public:
  using Id = uint64_t;
  using Clock = std::map<Id, Time>;

  struct Entry
  {
    Amount value;
    Clock alters;
    friend bool operator==(const Entry& l, const Entry& r)
    {
      return std::tie(l.value, l.alters) == std::tie(r.value, r.alters);
    }
    bool valid() const
    {
      return alters.size() > 0 && alters.begin()->first != 0UL;
    }
  };

  using JournalEntries = std::map<Clock, Entry>;

  Journal();
  explicit Journal(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;
  static Clock merge(const Clock& a, const Clock& b);
  static bool smaller(const Clock& a, const Clock& b);

  bool append(Amount value);
  bool append(Entry value);
  bool append(Id journalId, Amount value);
  bool append(Id journalId, Entry value);

  bool replace(Amount value, const Clock& clock);
  bool replace(Id journalId, Amount value, const Clock& clock);

  bool erase(Clock time);
  bool erase(Id journalId, Clock time);

  Entry query(Clock time) const;

  const JournalEntries& entries() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  JournalEntries _entries;
  Clock _clock;
};

using JournalPtr = std::shared_ptr<Journal>;

}

#endif
