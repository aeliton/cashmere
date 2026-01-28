include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable(type_tests)

# To be able to test plugin loading, the search will be relative to the
# executable: <exec path>/../lib/cashmere/plugins
set_target_properties(type_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

target_sources(type_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_clock.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_entry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_ledger.cpp
)

target_link_libraries (type_tests
  GTest::gmock
  GTest::gtest_main
  cashmere::crdt
)

include(GoogleTest)
gtest_discover_tests(type_tests)
