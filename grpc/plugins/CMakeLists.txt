set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cashmere/plugins")

add_library(plugin_grpc_objects OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/grpc.cpp
)
set_target_properties(plugin_grpc_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(plugin_grpc_objects
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
)
target_link_libraries(plugin_grpc_objects
  PUBLIC
  cashmere::cashmere
  cashmere::grpc_utils
)

add_library(grpc MODULE)

target_sources(grpc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/grpc_plugin.cpp
)
target_link_libraries(grpc PRIVATE
  $<BUILD_LOCAL_INTERFACE:plugin_grpc_objects>
)
set_target_properties(grpc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
  INSTALL_RPATH "$ORIGIN/../.."
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins)
install(TARGETS grpc DESTINATION ${PLUGIN_INSTALL_DIR})
