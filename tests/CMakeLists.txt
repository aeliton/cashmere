include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

add_executable(cli_tests)

target_sources(cli_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_command.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_options.cpp
)

target_include_directories(cli_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../cashmere
  ${CMAKE_CURRENT_SOURCE_DIR}/../cashmere/src
)

target_link_libraries (cli_tests
  GTest::gmock
  GTest::gtest_main
  cashmere::cashmere
  cashmere::cashmere_utils
  options_objects
)

include(GoogleTest)
gtest_discover_tests(cli_tests)
