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
#include <utility>

namespace Cashmere
{

IdConnectionInfoMap UpdateProvides(IdConnectionInfoMap provides);

struct Context
{
  BrokerIWeakPtr journal;
  Clock version;
  Port port;
  IdConnectionInfoMap provides;
};

BrokerI::~BrokerI() = default;

Broker::Broker()
{
  _contexts.push_back(std::make_shared<Context>());
}

Broker::~Broker() = default;

bool Broker::connect(BrokerIPtr remote)
{
  if (!remote) {
    return false;
  }
  auto local = getLocalPortFor(remote);

  auto context = _contexts.at(local);

  context->port = remote->getLocalPortFor(ptr());

  auto otherEntries = remote->entries(context->version, context->port);
  auto thisEntries = this->entries(context->version, local);

  remote->connect(ptr(), context->port, local);
  connect(remote, local, context->port);

  if (auto clock = remote->insert(thisEntries, context->port); clock.valid()) {
    context->version = clock;
    context->provides = UpdateProvides(remote->provides());
  }
  insert(otherEntries, local);

  return true;
}

Clock Broker::insert(const Entry& data, Port port)
{
  if (port < 0 || port >= _contexts.size()) {
    return Clock();
  }
  auto& ctx = _contexts[port];

  setClock(clock().merge(data.clock));

  if (auto j = ctx->journal.lock()) {
    ctx->provides = UpdateProvides(j->provides(ctx->port));
  } else {
    ctx->provides[data.entry.id].distance = 1;
  }
  ctx->provides[data.entry.id].version = clock();

  for (size_t i = 0; i < _contexts.size(); ++i) {
    if (i == port) {
      continue;
    }
    auto ctx = _contexts[i];
    if (ctx->provides.find(data.entry.id) != ctx->provides.cend()) {
      continue;
    }
    if (auto journal = ctx->journal.lock()) {
      if (auto clock = journal->insert(data, ctx->port); clock.valid()) {
        ctx->version = clock;
        ctx->provides = UpdateProvides(journal->provides());
      }
    }
  }
  return clock();
}

Clock Broker::insert(const EntryList& entries, Port port)
{
  for (auto& entry : entries) {
    insert(entry, port);
  }
  return clock();
};

IdConnectionInfoMap Broker::provides(Port to) const
{
  IdConnectionInfoMap out;
  for (size_t i = 0; i < _contexts.size(); i++) {
    if (i == to) {
      continue;
    }
    auto& ctx = _contexts[i];
    if (ctx->journal.lock()) {
      for (auto& [id, data] : ctx->provides) {
        auto it = std::as_const(out).find(id);
        if (it == out.cend() || data.distance < it->second.distance) {
          out[id] = data;
        }
      }
    }
  }
  return out;
}

EntryList Broker::entries(const Clock& from, Port ignore) const
{
  if (id() > 0) {
    return entries(from);
  }
  for (size_t i = 1; i < _contexts.size(); i++) {
    auto context = _contexts[i];
    if (i == ignore) {
      continue;
    }
    if (auto broker = context->journal.lock()) {
      return broker->entries(from, context->port);
    }
  }
  return {};
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

bool Broker::disconnect(Port port)
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

Clock Broker::clock() const
{
  return _contexts.front()->version;
}

EntryList Broker::entries(const Clock& from) const
{
  return entries(from, -1);
}

BrokerIPtr Broker::ptr()
{
  return this->shared_from_this();
}

void Broker::setClock(const Clock& clock)

{
  _contexts.front()->version = clock;
}

void Broker::connect(BrokerIPtr source, Port local, Port remote)
{
  auto context = _contexts.at(local);
  context->journal = source;
  context->port = remote;
  context->provides = UpdateProvides(source->provides());
}

Port Broker::getLocalPortFor(BrokerIPtr remote)
{
  Port port = _contexts.size();
  const auto remoteProvides = UpdateProvides(remote->provides());
  for (size_t i = 1; i < _contexts.size(); ++i) {
    auto localData = _contexts[i]->provides;
    auto remoteData = remoteProvides;
    while (remoteData.size() > 0 && localData.size() > 0) {
      const auto& [rId, rData] = *remoteData.begin();
      const auto& [lId, lData] = *localData.begin();
      if (rId == lId && rData.distance == lData.distance) {
        localData.erase(lId);
      }
      remoteData.erase(rId);
    }
    if (remoteData.empty() && localData.empty()) {
      port = i;
      break;
    }
  }
  if (port == _contexts.size()) {
    _contexts.push_back(std::make_shared<Context>());
  }
  return port;
}

bool ConnectionInfo::operator==(const ConnectionInfo& other) const
{
  return std::tie(distance, version) == std::tie(other.distance, other.version);
}

bool ConnectionInfo::operator<(const ConnectionInfo& other) const
{
  return std::tie(distance, version) < std::tie(other.distance, other.version);
}

IdConnectionInfoMap UpdateProvides(IdConnectionInfoMap provides)
{
  for (auto& [id, data] : provides) {
    ++data.distance;
  }
  return provides;
}

Id Broker::id() const
{
  return 0;
}
}
