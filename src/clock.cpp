#include "clock.h"

namespace Cashmere
{

Clock::Clock()
  : std::map<Id, Time>()
{
}

Clock::Clock(const std::initializer_list<std::pair<const Id, Time>>& list)
  : std::map<Id, Time>(list)
{
  std::erase_if(*this, [](const auto& item) { return item.second == 0; });
}

Clock Clock::merge(const Clock& other) const
{
  Clock out = *this;
  for (auto& [id, count] : other) {
    out[id] = std::max(find(id) != end() ? at(id) : 0, count);
  }
  return out;
}

bool Clock::smallerThan(const Clock& other) const
{
  return merge(other) == other;
}

bool Clock::concurrent(const Clock& other) const
{
  return !smallerThan(other) && !other.smallerThan(*this);
}

}
