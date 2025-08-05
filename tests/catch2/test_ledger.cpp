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
#include "cashmere/ledger.h"
#include <catch2/catch_all.hpp>

using namespace Cashmere;

struct ExistingRows
{
  const ClockEntryMap rows = {
    {Clock{{0xAA, 1}}, {Clock{{0xBB, 1}}, Data{0xBB, 100, Clock{{0xAA, 1}}}}}
  };
};

using Action = Ledger::Action;
using ActionClock = Ledger::ActionClock;

TEST_CASE("insert type transaction are ignored", "[replaces]")
{
  Entry existing = {{{0xAA, 1}}, {0xAA, 1, {}}};
  Entry incoming = {{{0xAA, 1}, {0xBB, 1}}, {0xBB, 10, {}}};
  REQUIRE(
    Ledger::Replaces(existing, incoming) == ActionClock{Action::Ignore, {}}
  );
}

TEST_CASE_METHOD(ExistingRows, "ignore old insert type entry", "[Evaluate]")
{
  Entry incoming = {{{0xAA, 1}}, {0xAA, 1, {}}};
  REQUIRE(Ledger::Evaluate(rows, incoming) == ActionClock{Action::Ignore, {}});
}

TEST_CASE_METHOD(ExistingRows, "insert if not present", "[Evaluate]")
{
  SECTION("insert type entry")
  {
    const Entry incoming = {{{0xAA, 2}}, {0xAA, 20, {}}};
    REQUIRE(
      Ledger::Evaluate(rows, incoming) ==
      ActionClock{Action::Insert, {{0xAA, 2}}}
    );
  }
  SECTION("edit type entry")
  {
    const Entry incoming = {{{0xBB, 1}}, {0xBB, 20, {{0xAA, 2}}}};
    REQUIRE(
      Ledger::Evaluate(rows, incoming) ==
      ActionClock{Action::Insert, {{0xAA, 2}}}
    );
  }
}

TEST_CASE("same journal has two appends", "[balance]")
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xFF, 2}}, Data{0xFF, 200, Clock{}}}
  };
  REQUIRE(Ledger::Balance(entries) == 500);
}

TEST_CASE("a different node edit's another node's entry", "[balance]")
{
  const auto entries = EntryList{
    {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
    {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}}
  };
  REQUIRE(Ledger::Balance(entries) == 50);
}

TEST_CASE("greater clock edit transactions wins", "[balance]")
{
  SECTION("samve id edits")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
      {Clock{{0xFF, 2}}, Data{0xFF, 200, Clock{}}},
      {Clock{{0xFF, 3}}, Data{0xFF, 0, Clock{{0xFF, 2}}}}
    };
    REQUIRE(Ledger::Balance(entries) == 300);
  }

  SECTION("smaller id edits bigger id once")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
      {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
    };
    REQUIRE(Ledger::Balance(entries) == 50);
  }

  SECTION("smaller id edits bigger id twice")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
      {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
      {Clock{{0xAA, 2}, {0xFF, 1}}, Data{0xAA, 25, Clock{{0xFF, 1}}}}
    };
    REQUIRE(Ledger::Balance(entries) == 25);
  }

  SECTION("smaller id edits bigger id once after previous edit")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 100, Clock{}}},
      {Clock{{0xFF, 2}}, Data{0xFF, 200, Clock{{0xFF, 1}}}},
      {Clock{{0xAA, 1}, {0xFF, 2}}, Data{0xAA, 300, Clock{{0xFF, 1}}}}
    };
    REQUIRE(Ledger::Balance(entries) == 300);
  }
}

TEST_CASE("greater id wins on conflicting edits", "[balance]")
{
  SECTION("processing smaller id conflict first")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
      {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}},
      {Clock{{0xFF, 2}}, Data{0xFF, 10, Clock{{0xFF, 1}}}}
    };
    REQUIRE(Ledger::Balance(entries) == 10);
  }

  SECTION("processing greater id conflict first")
  {
    const auto entries = EntryList{
      {Clock{{0xFF, 1}}, Data{0xFF, 300, Clock{}}},
      {Clock{{0xFF, 2}}, Data{0xFF, 10, Clock{{0xFF, 1}}}},
      {Clock{{0xAA, 1}, {0xFF, 1}}, Data{0xAA, 50, Clock{{0xFF, 1}}}}
    };
    REQUIRE(Ledger::Balance(entries) == 10);
  }
}
