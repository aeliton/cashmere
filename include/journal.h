#ifndef CASHEMERE_DEVICE_H
#define CASHEMERE_DEVICE_H

#include <cstdint>
#include <map>

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
    struct Id
    {
      Journal::Id journalId;
      Time time;
    };

    Operation operation;
    Amount value;
    Id alters;
  };

  using JournalEntries = std::map<Id, Entry>;
  using JournalBook = std::map<Id, JournalEntries>;

  Journal();
  explicit Journal(Id poolId);

  const Id id() const;
  const Id bookId() const;
  Clock clock() const;

  bool append(Amount value);
  bool append(const Entry& value);
  bool append(Id journalId, Amount value);
  bool append(Id journalId, const Entry& value);
  bool insert(Id journalId, Time time, Amount value);
  bool insert(Id journalId, Time time, const Entry& value);

  const Entry& query(Time time) const;
  const Entry& query(Id journalId, Time time) const;

  const JournalBook& journals() const;

private:
  static Random _random;
  const Id _bookId;
  const Id _id;
  JournalBook _book;
};

}

#endif
