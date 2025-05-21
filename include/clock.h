#ifndef CASHMERE_CLOCK_H
#define CASHMERE_CLOCK_H

#include <map>

#include "cashmere.h"

namespace Cashmere
{

class Clock : public std::map<Id, Time>
{
public:
  Clock();
  Clock(const std::initializer_list<std::pair<const Id, Time>>& list);
  Clock merge(const Clock& other) const;
  bool smallerThan(const Clock& other) const;
  bool concurrent(const Clock& other) const;
};

}
#endif
