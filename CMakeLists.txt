# Cashmere - a distributed conflict-free replicated database.
# Copyright (C) 2025 Aeliton G. Silva
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 3.30)

project(cash VERSION 0.0.1)

include(GNUInstallDirs)

add_subdirectory(signal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(cash_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/ledger.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/options.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/random.cpp
)
target_link_libraries(cash_objects PUBLIC signal)

target_include_directories(cash_objects PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(cash)
target_sources(cash PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(cash PRIVATE cash_objects)

find_package(Catch2 3 REQUIRED)

add_executable(catch_tests)
target_sources(catch_tests PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_ledger.cpp
)
target_link_libraries(catch_tests PRIVATE
  Catch2::Catch2WithMain
  cash_objects
)

include(CTest)
include(Catch)
catch_discover_tests(catch_tests)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable (gtests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_brokerhub.cpp
)
target_link_libraries (gtests
  cash_objects
  GTest::gmock
  GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(gtests)
