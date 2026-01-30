include(CashmerePluginsFunctions)

add_cashmere_plugin(grpc
  COMPILE_DEFINITIONS
    CASHMERE_GRPC_BUILD_PLUGIN
  LINK_LIBRARIES
    cashmere::cashmere
    cashmere::grpc_utils
)

add_library(plugin_grpc_objects OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/src/grpc.cpp
)
target_include_directories(plugin_grpc_objects PUBLIC
  $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(plugin_grpc_objects PUBLIC
  cashmere::cashmere
  cashmere::grpc_utils
)
