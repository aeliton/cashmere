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

#include "journal.h"
// #include <unordered_map>

namespace Cashmere
{

class Broker
{
public:
  Broker();

  bool attach(JournalPtr journal);
  bool detach(Id journalId);

  void onClockUndate(Id journalId, Clock clock);

  std::map<Id, Clock> versions() const;

private:
  std::unordered_map<Id, std::weak_ptr<Journal>> _attached;
  std::map<Id, Clock> _versions;
};

}

#endif
