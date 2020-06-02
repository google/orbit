cmake_minimum_required(VERSION 3.12)

project(test C CXX)

if(MSVC)
	add_definitions(-D_WIN32_WINNT=0x600)
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

list(PREPEND CMAKE_FIND_ROOT_PATH ${CONAN_OPENSSL_ROOT})

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(server)
target_sources(server PRIVATE
        greeter_server.cpp
        helloworld.proto)
grpc_helper(server)

add_executable(client)
target_sources(client PRIVATE
        greeter_client.cpp
        helloworld.proto)
grpc_helper(client)
