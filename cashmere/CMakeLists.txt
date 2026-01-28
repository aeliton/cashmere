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
cmake_minimum_required(VERSION 3.28)

project(cashmere VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

enable_testing()

add_subdirectory(proto)
add_subdirectory(utils)
add_subdirectory(types)
add_subdirectory(grpc_utils)
add_subdirectory(plugins)
add_subdirectory(tests)

add_library(cashmere_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbaseimpl.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerwrapperbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/cashmere.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/core.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcrunner.cpp
)

target_include_directories(cashmere_objects
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(cashmere_objects
  PUBLIC
    cashmere::types
    cashmere::cashmere_utils
    cashmere::proto
    cashmere::grpc_utils
)

set_target_properties(cashmere_objects PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

add_library(cashmere SHARED)

target_link_libraries(cashmere
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:cashmere_objects>
)

set_target_properties(cashmere PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
)

add_library(cashmere::cashmere ALIAS cashmere)

set_target_properties(cashmere PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

install(
  TARGETS proto cashmere_utils grpc_utils cashmere
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

list( APPEND CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_LIBDIR} )

