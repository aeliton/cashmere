include(CTest)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

enable_testing()
add_executable (gtests
  ${CMAKE_CURRENT_SOURCE_DIR}/test_broker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokergrpcstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_brokerstub.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_clock.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_command.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_entry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journal.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_journalfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_ledger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_options.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_urlutils.cpp
)

target_include_directories(gtests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries (gtests
  GTest::gmock
  GTest::gtest_main
  cashmere_objects
  cashmere::cashmere_utils
  options_objects
)

include(GoogleTest)
gtest_discover_tests(gtests)
