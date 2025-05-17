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
    Delete
  };

  struct Entry
  {
    Operation operation;
    Amount value;
    Clock alters;
  };

  using JournalEntries = std::map<Clock, Entry>;

  Journal();
  explicit Journal(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(const Entry& value);
  bool append(Id journalId, Amount value);
  bool append(Id journalId, const Entry& value);

  bool replace(Id journalId, const Clock& clock, Amount value);

  bool erase(Id journalId, const Clock& time);

  const Entry& query() const;
  const Entry& query(const Clock& time) const;

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
