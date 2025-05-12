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

project(cash)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(cash_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/device.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/options.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/random.cpp
)
target_include_directories(cash_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(cash)
target_sources(cash PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(cash PRIVATE cash_objects)

find_package(Catch2 3 REQUIRED)

add_executable(tests)
target_sources(tests PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_device.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_device_clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_device_id.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_options.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain cash_objects)

include(CTest)
include(Catch)
catch_discover_tests(tests)
