cmake_minimum_required(VERSION 3.15)

project(OrbitProtos)

add_library(OrbitProtos STATIC)
target_compile_options(OrbitProtos PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_definitions(OrbitProtos PRIVATE -D_WIN32_WINNT=0x0700)
target_compile_definitions(OrbitProtos PRIVATE -DNTDDI_VERSION=0x06030000)

target_sources(OrbitProtos PRIVATE code_block.proto
                                   module.proto
                                   process.proto
                                   services.proto
                                   symbol.proto)

grpc_helper(OrbitProtos)


set(OrbitFileProtoList config.proto)
set(OrbitFileProtoSources config.pb.cc)
set(OrbitFileProtoHeaders config.pb.h)
add_custom_command(
    OUTPUT
      ${OrbitFileProtoSources} ${OrbitFileProtoHeaders}
    COMMAND
      protoc ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}" ${OrbitFileProtoList}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR})

add_library(OrbitFileProtos STATIC ${OrbitFileProtoSources})
target_link_libraries(OrbitFileProtos PUBLIC protobuf::libprotobuf)
target_include_directories(OrbitFileProtos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
