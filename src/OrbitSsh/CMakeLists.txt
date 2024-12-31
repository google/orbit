# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitSsh)

add_library(OrbitSsh STATIC)

target_include_directories(OrbitSsh PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  OrbitSsh
  PUBLIC include/OrbitSsh/AddrAndPort.h
         include/OrbitSsh/Channel.h
         include/OrbitSsh/Context.h
         include/OrbitSsh/Credentials.h
         include/OrbitSsh/Error.h
         include/OrbitSsh/KnownHostsError.h
         include/OrbitSsh/Session.h
         include/OrbitSsh/Sftp.h
         include/OrbitSsh/SftpFile.h
         include/OrbitSsh/Socket.h)

target_sources(
  OrbitSsh
  PRIVATE Channel.cpp
          Context.cpp
          Credentials.cpp
          Error.cpp
          HomeEnvironmentVariable.h
          KnownHostsError.cpp
          LibSsh2Utils.cpp
          Session.cpp
          Sftp.cpp
          SftpFile.cpp
          Socket.cpp)

if(WIN32)
  target_sources(OrbitSsh PRIVATE WindowsSocket.cpp)
else()
  target_sources(OrbitSsh PRIVATE LinuxSocket.cpp)
endif()

target_link_libraries(
  OrbitSsh
  PUBLIC OrbitBase
         Libssh2::libssh2)

if(WIN32)
  target_compile_definitions(OrbitSsh PUBLIC WIN32_LEAN_AND_MEAN)
  target_compile_definitions(OrbitSsh PUBLIC -D_WIN32_WINNT=0x0700)
  target_compile_definitions(OrbitSsh PUBLIC -DNTDDI_VERSION=0x06030000)
endif()

add_executable(OrbitSshTests)
target_sources(OrbitSshTests PRIVATE
  ContextTests.cpp
  CredentialsTest.cpp
  SocketTests.cpp)

target_link_libraries(OrbitSshTests PRIVATE
  OrbitSsh
  TestUtils
  GTest_Main)

register_test(OrbitSshTests PROPERTIES TIMEOUT 10)
