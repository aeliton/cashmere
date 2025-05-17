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

  enum class Operation
  {
    Insert,
    Replace,
    Delete,
    Invalid
  };

  struct Entry
  {
    Operation operation;
    Amount value;
    Clock alters;
    friend bool operator==(const Entry& l, const Entry& r)
    {
      return std::tie(l.operation, l.value, l.alters) ==
             std::tie(r.operation, r.value, r.alters);
    }
    bool valid() const
    {
      return operation != Operation::Invalid;
    }
  };

  using JournalEntries = std::map<Clock, Entry>;

  Journal();
  explicit Journal(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(Entry value);
  bool append(Id journalId, Amount value);
  bool append(Id journalId, Entry value);

  bool replace(Id journalId, const Clock& clock, Amount value);

  bool erase(Id journalId, const Clock& time);

  Entry query(Clock time) const;

  const JournalEntries& journals() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  JournalEntries _book;
  Clock _clock;
};

using JournalPtr = std::shared_ptr<Journal>;

}

#endif
