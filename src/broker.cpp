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
#include "broker.h"
#include <cassert>

namespace Cashmere
{

Context::Context(std::shared_ptr<Broker> j, const Clock& v, Connection c)
  : journal(j)
  , version(v)
  , conn(c)
  , provides({})
{
}

Broker::~Broker() = default;

Broker::Broker(Id id)
  : _id(id)
{
  _contexts.push_back(std::make_shared<Context>(nullptr, Clock{}, 0));
}

Id Broker::id() const
{
  return _id;
}

void Broker::setClock(const Clock& clock)
{
  _contexts.front()->version = clock;
}

void Broker::clockTick(Id id)
{
  _contexts.front()->version[id]++;
};

Clock Broker::clock() const
{
  return _contexts.front()->version;
}

ClockEntryList Broker::entries(const Clock& from) const
{
  for (const auto& context : _contexts) {
    if (!context->containsEntries()) {
      continue;
    }
    if (auto journal = context->journal.lock()) {
      return context->journal.lock()->entries(from);
    }
  }
  return {};
}

IdDistanceMap Broker::provides() const
{
  IdDistanceMap out;
  for (auto& ctx : _contexts) {
    if (ctx->journal.lock()) {
      auto provides = ctx->provides;
      out.merge(provides);
      assert(provides.empty());
    }
  }
  return out;
}

bool Broker::attach(BrokerPtr other)
{
  if (!other) {
    return false;
  }
  Port local = _contexts.size();
  for (size_t i = 1; i < _contexts.size(); ++i) {
    if (_contexts[i]->provides.empty()) {
      continue;
    }
    auto provides = other->provides();
    if (provides.find(_contexts[i]->provides.begin()->first) !=
        provides.end()) {
      local = i;
      break;
    }
  }

  ClockEntryList otherEntries;
  ClockEntryList thisEntries;
  auto remote = other->attach(ptr(), local);
  if (local == _contexts.size()) {
    otherEntries = other->entries();
    thisEntries = this->entries();
    attach(other, remote);
  } else {
    _contexts[local]->journal = other;
    otherEntries = other->entries(_contexts[local]->version);
    thisEntries = this->entries(_contexts[local]->version);
  }

  if (other->insert(thisEntries, remote)) {
    _contexts[local]->version = other->clock();
    _contexts[local]->provides = UpdateProvides(other->provides());
  }
  insert(otherEntries, local);

  return true;
}

Port Broker::attach(BrokerPtr source, Port port)
{
  _contexts.push_back(std::make_shared<Context>(source, source->clock(), port));
  _contexts.back()->provides = UpdateProvides(source->provides());
  for (auto& [id, dist] : _contexts.back()->provides) {
    _contextMap[id] = _contexts.back();
  }

  return _contexts.size() - 1;
}

bool Broker::detach(Port port)
{
  if (port < 0 || _contexts.size() <= port) {
    return false;
  }
  auto& context = _contexts.at(port);
  if (auto handler = context->journal.lock()) {
    context->journal.reset();
    return true;
  }
  return false;
}

BrokerPtr Broker::ptr()
{
  return this->shared_from_this();
}

bool Broker::insert(const ClockEntryList& entries, Port port)
{
  bool success = true;
  for (auto& entry : entries) {
    success &= insert(entry, port);
  }
  return success;
};

bool Broker::insert(const ClockEntry& data, Port port)
{
  if (port < 0 || port >= _contexts.size()) {
    return false;
  }
  auto& ctx = _contexts[port];

  ctx->provides[data.entry.journalId].version =
    ctx->provides[data.entry.journalId].version.merge(data.clock);

  for (size_t i = 0; i < _contexts.size(); ++i) {
    auto ctx = _contexts[i];
    if (ctx->provides.find(data.entry.journalId) != ctx->provides.cend()) {
      continue;
    }
    if (auto journal = ctx->journal.lock()) {
      if (journal->insert(data, ctx->conn)) {
        ctx->version = journal->clock();
        ctx->provides = UpdateProvides(journal->provides());
      }
    }
  }
  setClock(clock().merge(data.clock));
  return true;
}

IdClockMap Broker::versions() const
{
  IdClockMap out;
  for (auto& context : _contexts) {
    for (auto& [id, data] : context->provides) {
      out[id] = data.version;
    }
  }
  return out;
}

IdDistanceMap Broker::UpdateProvides(IdDistanceMap provides)
{
  for (auto& [id, data] : provides) {
    ++data.distance;
  }
  return provides;
}
}
