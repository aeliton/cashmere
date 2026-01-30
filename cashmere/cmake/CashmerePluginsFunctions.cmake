function(add_cashmere_plugin PLUGIN_NAME)
  set(LIST_ARGS COMPILE_DEFINITIONS LINK_LIBRARIES)
  cmake_parse_arguments(PARSE_ARGV 1 cash
    "" "" "${LIST_ARGS}"
  )
  add_library(${PLUGIN_NAME} MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/${PLUGIN_NAME}.cpp
  )
  target_compile_definitions(${PLUGIN_NAME} PRIVATE
    ${cash_COMPILE_DEFINITIONS}
  )
  target_include_directories(${PLUGIN_NAME} PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_link_libraries(${PLUGIN_NAME} PRIVATE
    cashmere::cashmere
    ${cash_LINK_LIBRARIES}
  )
  set_target_properties(${PLUGIN_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/../.."
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cashmere/plugins
  )
  install(
    TARGETS ${PLUGIN_NAME}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cashmere/plugins"
  )
endfunction()

