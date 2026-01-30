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
add_library(grpc_runner MODULE)

target_sources(grpc_runner PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcrunner.cpp
)

target_include_directories(grpc_runner
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(grpc_runner PRIVATE
  cashmere::grpc_utils
)

add_library(cashmere::grpc_runner ALIAS grpc_runner)

set_target_properties(grpc_runner PROPERTIES
  LIBRARY_OUTPUT_NAME grpc
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
  INSTALL_RPATH "$ORIGIN/../.."
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/wrappers
)
set(WRAPPER_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cashmere/wrappers")

install(TARGETS grpc_runner DESTINATION ${WRAPPER_INSTALL_DIR})
