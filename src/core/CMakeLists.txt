find_package(gRPC CONFIG REQUIRED)

add_library(cashmere_proto STATIC proto/cashmere.proto)
target_link_libraries(cashmere_proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cashmere_proto PUBLIC 
  $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET cashmere_proto LANGUAGE cpp)
protobuf_generate(
  TARGET cashmere_proto
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
  PLUGIN_OPTIONS "generate_mock_code=true"
)

add_library(cashmere_objects OBJECT
 ${CMAKE_CURRENT_SOURCE_DIR}/broker.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/brokerbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/brokerbaseimpl.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/brokergrpcstub.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/brokerwrapperbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/cashmere.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/clock.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/entry.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/grpcrunner.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/grpcutils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/journal.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/journalbase.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/journalfile.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/ledger.cpp
)

target_include_directories(cashmere_objects
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_LOCAL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(cashmere_objects
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:cashmere_proto>
)

set_target_properties(cashmere_objects cashmere_proto PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

add_library(cashmere SHARED)

target_link_libraries(cashmere
  PUBLIC
    $<BUILD_LOCAL_INTERFACE:cashmere_objects>
)

set_target_properties(cashmere PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
)

generate_export_header(cashmere EXPORT_FILE_NAME export/cashmere/cashmere_export.h)

add_library(cashmere::cashmere ALIAS cashmere)

