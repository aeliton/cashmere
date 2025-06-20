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
#ifndef CASHMERE_SIGNAL_H
#define CASHMERE_SIGNAL_H

#include <functional>
#include <map>

namespace Signaller
{
using Connection = size_t;

template<typename Function>
class Signal;

template<typename ReturnType, typename... Args>
class Signal<ReturnType(Args...)>
{
public:
  Signal() = default;

  using Slot = std::function<ReturnType(Args...)>;

  void operator()(Args... t) const
  {
    for (auto& [conn, slot] : _slots) {
      slot(t...);
    }
  }

  Connection connect(Slot slot)
  {
    _slots[++_count] = slot;
    return _count;
  }

  size_t count() const
  {
    return _count - _disconnected;
  }

  template<class Class, class Member = ReturnType (Class::*)(Args...)>
  Connection connect(Class* object, Member&& member)
  {
    return connect(std::bind_front(member, object));
  }

  bool disconnect(Connection connection)
  {
    if (_slots.find(connection) == _slots.end()) {
      return false;
    }
    _slots.erase(connection);
    _disconnected++;
    return true;
  }

private:
  std::map<Connection, Slot> _slots;
  Connection _count = 0;
  size_t _disconnected = 0;
};
}

#endif
