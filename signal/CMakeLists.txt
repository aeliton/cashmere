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

project(signal)

add_library(signal INTERFACE)
target_include_directories( signal INTERFACE
  $<BUILD_INTERFACE:${signal_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(signal INTERFACE cxx_std_20)
set_target_properties(signal
  PROPERTIES CXX_EXTENSIONS OFF
  CMAKE_COMPILE_WARNING_AS_ERROR ON
)

find_package(Catch2 3 REQUIRED)

add_executable(tests)
target_sources(tests PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_signal.cpp
)
target_link_libraries(tests PRIVATE
  Catch2::Catch2WithMain
  signal
)

include(CTest)
include(Catch)
catch_discover_tests(tests)
