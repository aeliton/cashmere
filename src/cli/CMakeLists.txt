add_library(options_objects OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/command.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/options.cpp
)

target_include_directories(options_objects
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(options_objects PRIVATE cashmere_objects)

add_executable(cash)

set_target_properties(cash PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

if (UNIX AND NOT CYGWIN)
  set_target_properties(cash PROPERTIES
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
  )
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(libedit REQUIRED libedit)

target_sources(cash PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_include_directories(cash
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(cash PRIVATE
  $<BUILD_LOCAL_INTERFACE:options_objects>
  cashmere::cashmere
  cashmere::cashmere_utils
  edit
)

