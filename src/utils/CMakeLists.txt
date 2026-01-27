add_library(cashmere_utils_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/fileutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/random.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/urlutils.cpp
)

target_include_directories(cashmere_utils_objects
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

add_library(cashmere_utils SHARED)
target_link_libraries(cashmere_utils
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:cashmere_utils_objects>
)
add_library(cashmere::cashmere_utils ALIAS cashmere_utils)

set_target_properties(cashmere_utils_objects PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)
