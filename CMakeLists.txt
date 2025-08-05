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

project(cashmere VERSION 0.0.1 LANGUAGES CXX)

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

add_library(cashmere_proto STATIC proto/cashmere.proto)
target_link_libraries(cashmere_proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cashmere_proto PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET cashmere_proto LANGUAGE cpp)
protobuf_generate(
  TARGET cashmere_proto
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
  PLUGIN_OPTIONS "generate_mock_code=true"
)

include(GenerateExportHeader)

add_library(cashmere_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokergrpc.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokergrpcclient.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokergrpcstub.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journalbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/journalfile.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/ledger.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/fileutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/grpcutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/random.cpp
)

target_include_directories(cashmere_objects
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(cashmere_objects
  PUBLIC
    $<BUILD_INTERFACE:cashmere_proto>
)

set_target_properties(cashmere_objects cashmere_proto PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

add_library(cashmere SHARED)

target_link_libraries(cashmere
  PUBLIC
    $<BUILD_INTERFACE:cashmere_objects>
)

set_target_properties(cashmere PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
)

generate_export_header(cashmere EXPORT_FILE_NAME export/cashmere/cashmere_export.h)

add_library(cashmere::cashmere ALIAS cashmere)

install(
  TARGETS cashmere
  EXPORT CashmereTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/export/cashmere/cashmere_export.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cashmere
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME}
)

install(
  EXPORT CashmereTargets
  FILE CashmereTargets.cmake
  NAMESPACE cashmere::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/cashmere
)

add_library(options_objects OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/src/command.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/options.cpp
)
target_link_libraries(options_objects PRIVATE cashmere_objects)

add_executable(cash)
target_sources(cash PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
target_include_directories(cash
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(cash PRIVATE options_objects cashmere_objects)

add_library(test_helper INTERFACE)
target_include_directories(test_helper INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/tests/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(test_helper INTERFACE cashmere_objects)

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
  cashmere_objects
  test_helper
  signal
)
target_include_directories(catch_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(CTest)
include(Catch)
catch_discover_tests(catch_tests)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable (gtests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_broker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_brokergrpcstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_command.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_journal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_journalfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/gtest/test_options.cpp
)

target_include_directories(gtests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries (gtests
  GTest::gmock
  GTest::gtest_main
  cashmere_objects
  options_objects
  test_helper
  cashmere_proto
)

include(GoogleTest)
gtest_discover_tests(gtests)
