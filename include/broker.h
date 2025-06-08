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
#ifndef CASHMERE_BROKER_H
#define CASHMERE_BROKER_H

#include "entry.h"
#include <unordered_map>

namespace Cashmere
{

struct Context
{
  Context(std::shared_ptr<EntryHandler> j, const Clock& v, Connection c);
  std::weak_ptr<EntryHandler> journal;
  Clock version;
  Connection conn;
};

using ContextPtr = std::shared_ptr<Context>;

class Broker : public EntryHandler
{
public:
  Broker();

  bool insert(const ClockEntry& data) override;

  bool attach(EntryHandlerPtr journal);
  bool detach(Id journalId);

  IdSet provides() const override;

  IdClockMap versions() const;

private:
  EntryHandlerPtr pickAttached() const;

  std::vector<ContextPtr> _attached;
  std::unordered_map<Id, ContextPtr> _idToContext;
};

}

#endif
