include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable(gtests)

# To be able to test plugin loading, the search will be relative to the
# executable: <exec path>/../lib/cashmere/plugins
set_target_properties(gtests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

target_sources(gtests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_broker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokergrpcstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journalfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_plugins.cpp
)

target_include_directories(gtests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries (gtests
  GTest::gmock
  GTest::gtest_main
  cashmere::cashmere
  cashmere::cashmere_utils
  options_objects
  $<BUILD_LOCAL_INTERFACE:plugin_grpc_objects>
)

include(GoogleTest)
gtest_discover_tests(gtests)
