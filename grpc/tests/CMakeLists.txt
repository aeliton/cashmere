include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable(cashmere_grpc_tests)

# To be able to test plugin loading, the search will be relative to the
# executable: <exec path>/../lib/cashmere/plugins
set_target_properties(cashmere_grpc_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

target_sources(cashmere_grpc_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokergrpcstub.cpp
)

target_include_directories(cashmere_grpc_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries (cashmere_grpc_tests
  GTest::gmock
  GTest::gtest_main
  $<BUILD_LOCAL_INTERFACE:plugin_grpc_objects>
)

include(GoogleTest)
gtest_discover_tests(cashmere_grpc_tests)
