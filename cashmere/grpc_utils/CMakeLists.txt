add_library(grpc_utils SHARED)

target_sources(grpc_utils PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcutils.cpp
)

target_include_directories(grpc_utils
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(grpc_utils PRIVATE
  cashmere::types
  cashmere::proto
)

set_target_properties(grpc_utils PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
)

add_library(cashmere::grpc_utils ALIAS grpc_utils)
