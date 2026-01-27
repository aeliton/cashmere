set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cashmere/plugins")

add_library(hub MODULE
  ${CMAKE_CURRENT_SOURCE_DIR}/hub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hub_plugin.cpp
)
set_target_properties(hub PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(hub PRIVATE cashmere::cashmere)
set_target_properties(hub PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins)
install(TARGETS hub DESTINATION ${PLUGIN_INSTALL_DIR})

add_library(cache MODULE
  ${CMAKE_CURRENT_SOURCE_DIR}/cache.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cache_plugin.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/journalbase.cpp
)
set_target_properties(cache PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(cache PRIVATE cashmere::cashmere)
set_target_properties(cache PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins)
install(TARGETS cache DESTINATION ${PLUGIN_INSTALL_DIR})

add_library(file MODULE
  ${CMAKE_CURRENT_SOURCE_DIR}/file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/file_plugin.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/journalbase.cpp
)
set_target_properties(file PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(file PRIVATE cashmere::cashmere)
set_target_properties(file PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins)
install(TARGETS file DESTINATION ${PLUGIN_INSTALL_DIR})
target_include_directories(file
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
)

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
)

add_library(grpc MODULE)

target_sources(grpc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/grpc_plugin.cpp
)
set_target_properties(grpc PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(grpc PRIVATE
  $<BUILD_LOCAL_INTERFACE:plugin_grpc_objects>
)
set_target_properties(grpc PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins)
install(TARGETS grpc DESTINATION ${PLUGIN_INSTALL_DIR})
