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
#include "entry.h"

namespace Cashmere
{

Context::Context(std::shared_ptr<EntryHandler> j, const Clock& v, Connection c)
  : journal(j)
  , version(v)
  , conn(c)
  , provides({})
{
}

EntryHandler::~EntryHandler() = default;

const bool ClockEntry::operator==(const ClockEntry& other) const
{
  return clock == other.clock && entry == other.entry;
}

EntryHandler::EntryHandler(Id id)
  : _id(id)
{
  _contexts.push_back(std::make_shared<Context>(nullptr, Clock{}, 0));
}

Id EntryHandler::id() const
{
  return _id;
}

void EntryHandler::setClock(const Clock& clock)
{
  _contexts.front()->version = clock;
}

void EntryHandler::clockTick(Id id)
{
  _contexts.front()->version[id]++;
};

Clock EntryHandler::clock() const
{
  return _contexts.front()->version;
}

ClockEntryList EntryHandler::entries(const Clock& from) const
{
  for (const auto& context : _contexts) {
    if (context->provides.size() == 1 &&
        context->provides.begin()->second.distance == 0) {
      return context->journal.lock()->entries(from);
    }
  }
  return {};
}

IdDistanceMap EntryHandler::provides() const
{
  IdDistanceMap out;
  for (auto& ctx : _contexts) {
    if (ctx->journal.lock()) {
      for (auto& [id, dist] : ctx->provides) {
        out[id].distance = dist.distance + 1;
        out[id].version = dist.version;
      }
    }
  }
  return out;
}

bool EntryHandler::attach(EntryHandlerPtr other)
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
    _contexts[local]->provides = other->provides();
  }
  insert(otherEntries, local);

  return true;
}

Port EntryHandler::attach(EntryHandlerPtr source, Port port)
{
  _contexts.push_back(std::make_shared<Context>(source, source->clock(), port));
  _contexts.back()->provides = source->provides();
  for (auto& [id, dist] : _contexts.back()->provides) {
    _contextMap[id] = _contexts.back();
  }

  return _contexts.size() - 1;
}

bool EntryHandler::detach(Port port)
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

EntryHandlerPtr EntryHandler::ptr()
{
  return this->shared_from_this();
}

bool EntryHandler::insert(const ClockEntryList& entries, Port port)
{
  bool success = true;
  for (auto& entry : entries) {
    success &= insert(entry, port);
  }
  return success;
};

bool EntryHandler::insert(const ClockEntry& data, Port port)
{
  if (port < 0 || port >= _contexts.size()) {
    return false;
  }
  auto& ctx = _contexts[port];

  if (_contextMap.find(data.entry.journalId) == _contextMap.cend()) {
    _contextMap[data.entry.journalId] = ctx;
    ctx->provides[data.entry.journalId].distance = 0;
    ctx->provides[data.entry.journalId].version = data.clock;
  }

  for (size_t i = 0; i < _contexts.size(); ++i) {
    auto ctx = _contexts[i];
    if (ctx->provides.find(data.entry.journalId) != ctx->provides.cend()) {
      continue;
    }
    if (auto journal = ctx->journal.lock()) {
      if (journal->insert(data, ctx->conn)) {
        ctx->version = journal->clock();
        ctx->provides = journal->provides();
      }
    }
  }
  setClock(clock().merge(data.clock));
  return true;
}

IdClockMap EntryHandler::versions() const
{
  IdClockMap out;
  for (auto& context : _contexts) {
    for (auto& [id, data] : context->provides) {
      out[id] = data.version;
    }
  }
  return out;
}

}
