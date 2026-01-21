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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "utils/urlutils.h"

using namespace Cashmere;

class StringToUrlTest
  : public ::testing::TestWithParam<std::tuple<std::string, Url>>
{
};

TEST_P(StringToUrlTest, FromString)
{
  const auto [input, expected] = GetParam();
  ASSERT_EQ(ParseUrl(input), expected);
}

INSTANTIATE_TEST_SUITE_P(
  Url, StringToUrlTest,
  ::testing::Values(
    std::tuple<std::string, Url>{"ssh://localhost", Url{"ssh://localhost", "ssh", "", "localhost", "" }},
    std::tuple<std::string, Url>{"ssh://localhost", Url{"ssh://localhost", "ssh", "", "localhost", "" }},
    std::tuple<std::string, Url>{"ssh://user@host", Url{"ssh://user@host", "ssh", "user", "host", "" }},
    std::tuple<std::string, Url>{"ssh://u:p@h:p", Url{"ssh://u:p@h:p", "ssh", "u:p", "h:p", "" }},
    std::tuple<std::string, Url>{"ssh://u:p@h:p/pa/th", Url{"ssh://u:p@h:p/pa/th", "ssh", "u:p", "h:p", "/pa/th" }},
    std::tuple<std::string, Url>{"ssh://", Url{"ssh://", "ssh", "", "", "" }},
    std::tuple<std::string, Url>{"invalidurl", Url{"invalidurl", "", "", "", "" }}
  )
);
