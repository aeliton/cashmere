add_subdirectory(utils)
add_subdirectory(types)
add_subdirectory(plugins)
add_subdirectory(tests)

add_library(cashmere_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerbaseimpl.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerstore.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/brokerwrapperbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/cashmere.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/src/wrapperstore.cpp
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

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
