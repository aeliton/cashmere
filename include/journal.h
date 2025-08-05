// Cashmere - a distributed conflict-free replicated database.
// Copyright (C) 2025 Aeliton G. Silva
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#ifndef CASHEMERE_JOURNAL_H
#define CASHEMERE_JOURNAL_H

#include "journalbase.h"

namespace Cashmere
{
class CASHMERE_EXPORT Journal : public JournalBase
{
public:
  explicit Journal();
  explicit Journal(Id id, const ClockDataMap& entries = {});

  bool save(const Entry& data) override;
  Data entry(Clock time) const override;
  EntryList entries() const override;

private:
  ClockDataMap _entries;
};

}

#endif
