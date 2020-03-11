cmake_minimum_required(VERSION 2.8.12)
project(cmake_wrapper)

include(conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_program(_gRPC_PROTOBUF_PROTOC_EXECUTABLE protoc)
set(_gRPC_PROTOBUF_PROTOC_LIBRARIES CONAN_PKG::protobuf)
set(_gRPC_PROTOBUF_LIBRARIES CONAN_PKG::protobuf)
get_target_property(_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR CONAN_PKG::protobuf INTERFACE_INCLUDE_DIRECTORIES)
list(GET _gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR 0 _gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR)

set(OPENSSL_ROOT_DIR "${CONAN_OPENSSL_ROOT}")
list(APPEND CMAKE_FIND_ROOT_PATH "${CONAN_OPENSSL_ROOT}")

add_subdirectory("source_subfolder")
