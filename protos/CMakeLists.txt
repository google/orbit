cmake_minimum_required(VERSION 3.15)

project(OrbitProtos)

add_library(OrbitProtos STATIC)
target_compile_options(OrbitProtos PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitProtos PRIVATE dummy.cpp
                                   process.proto
                                   services.proto)

grpc_helper(OrbitProtos)

