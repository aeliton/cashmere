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

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_library(cashmere_proto proto/cashmere.proto)
target_link_libraries(cashmere_proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cashmere_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET cashmere_proto LANGUAGE cpp)
protobuf_generate(
  TARGET cashmere_proto
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
  PLUGIN_OPTIONS "generate_mock_code=true"
)

add_library(cash_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokergrpc.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokergrpcstub.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journalbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journalfile.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/ledger.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/options.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/random.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/fileutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/grpcutils.cpp
)
target_link_libraries(cash_objects PUBLIC cashmere_proto)

target_include_directories(cash_objects PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(cash)
target_sources(cash PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(cash PRIVATE cash_objects)

add_library(test_helper INTERFACE)
target_include_directories(test_helper INTERFACE
  $<BUILD_INTERFACE:${cash_SOURCE_DIR}/tests/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(test_helper INTERFACE cash_objects)

find_package(Catch2 3 REQUIRED)

add_executable(catch_tests)
target_sources(catch_tests PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_entry.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_journalfile.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch2/test_ledger.cpp
)
target_link_libraries(catch_tests PRIVATE
  Catch2::Catch2WithMain
  cash_objects
  test_helper
  signal
)

include(CTest)
include(Catch)
catch_discover_tests(catch_tests)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable (gtests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_broker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_brokergrpcstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_journal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_journalfile.cpp
)
target_link_libraries (gtests
  GTest::gmock
  GTest::gtest_main
  cash_objects
  test_helper
  cashmere_proto
)

include(GoogleTest)
gtest_discover_tests(gtests)
