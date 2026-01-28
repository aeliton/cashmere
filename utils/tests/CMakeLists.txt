include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

add_executable(utils_tests)

set_target_properties(utils_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

target_sources(utils_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_file.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_url.cpp
)

target_link_libraries (utils_tests
  GTest::gmock
  GTest::gtest_main
  cashmere::cashmere_utils
)

include(GoogleTest)
gtest_discover_tests(utils_tests)
