set(WRAPPER_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cashmere/wrappers")

enable_testing()

add_subdirectory(proto)
add_subdirectory(plugins)
add_subdirectory(tests)


add_library(grpc_utils SHARED)

target_sources(grpc_utils PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcrunner.cpp
)

target_include_directories(grpc_utils
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(grpc_utils PUBLIC
  cashmere::cashmere
  cashmere::cashmere_utils
  cashmere::proto
)

add_library(cashmere::grpc_utils ALIAS grpc_utils)

set_target_properties(grpc_utils PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
)

add_library(grpc_runner MODULE)

target_sources(grpc_runner PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcrunner.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/grpcrunner_plugin.cpp
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
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/wrappers
)
install(TARGETS grpc_runner DESTINATION ${WRAPPER_INSTALL_DIR})
