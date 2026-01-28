include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

add_executable(cashmere_tests)

# To be able to test plugin loading, the search will be relative to the
# executable: <exec path>/../lib/cashmere/plugins
set_target_properties(cashmere_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

target_sources(cashmere_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_broker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journalfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_plugins.cpp
)

target_include_directories(cashmere_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries (cashmere_tests
  GTest::gmock
  GTest::gtest_main
  cashmere::cashmere
  cashmere::cashmere_utils
)

include(GoogleTest)
gtest_discover_tests(cashmere_tests)
