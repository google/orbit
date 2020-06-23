cmake_minimum_required(VERSION 3.15)

project(OrbitProtos)

add_library(OrbitProtos STATIC)
target_compile_options(OrbitProtos PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_definitions(OrbitProtos PRIVATE -D_WIN32_WINNT=0x0700)
target_compile_definitions(OrbitProtos PRIVATE -DNTDDI_VERSION=0x06030000)

target_sources(OrbitProtos PRIVATE code_block.proto
                                   module.proto
                                   process.proto
                                   services.proto)

grpc_helper(OrbitProtos)
