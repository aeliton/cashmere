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
#include <vector>

namespace Cashmere
{
template<typename R, typename... T>
class Signal
{
public:
  Signal() = default;

  using Slot = std::function<R(T...)>;

  void operator()(T... t) const
  {
    for (auto& callback : _callbacks) {
      callback(t...);
    }
  }

  bool connect(Slot functor)
  {
    _callbacks.push_back(functor);
    return true;
  }

private:
  std::vector<Slot> _callbacks;
};
}

#endif
