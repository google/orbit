# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitSsh)

add_library(OrbitSsh STATIC)

target_compile_options(OrbitSsh PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitSsh PUBLIC cxx_std_17)

target_include_directories(OrbitSsh PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitSsh PUBLIC
        include/OrbitSsh/Channel.h
        include/OrbitSsh/DirectTcpIpChannelManager.h
        include/OrbitSsh/ExecChannelManager.h
        include/OrbitSsh/LocalSocketManager.h
        include/OrbitSsh/ResultType.h
        include/OrbitSsh/Session.h
        include/OrbitSsh/SessionManager.h
        include/OrbitSsh/SshManager.h
        include/OrbitSsh/Socket.h
        include/OrbitSsh/TunnelManager.h)

target_sources(OrbitSsh PRIVATE
        Channel.cpp
        DirectTcpIpChannelManager.cpp
        ExecChannelManager.cpp
        LocalSocketManager.cpp
        Session.cpp
        SessionManager.cpp
        SshManager.cpp
        Socket.cpp
        TunnelManager.cpp)

if(WIN32)
  target_sources(OrbitSsh PRIVATE WindowsSocket.cpp)
else()
  target_sources(OrbitSsh PRIVATE LinuxSocket.cpp)
endif()

target_link_libraries(OrbitSsh PUBLIC OrbitBase libssh2::libssh2)

if(WIN32)
  target_compile_definitions(OrbitSsh PUBLIC WIN32_LEAN_AND_MEAN)
  target_compile_definitions(OrbitSsh PUBLIC -D_WIN32_WINNT=0x0700)
  target_compile_definitions(OrbitSsh PUBLIC -DNTDDI_VERSION=0x06030000)
endif()

add_executable(OrbitSshTests)

target_sources(OrbitSshTests PRIVATE SocketTests.cpp)

target_link_libraries(OrbitSshTests PRIVATE OrbitSsh libssh2::libssh2
                                            GTest::Main)

register_test(OrbitSshTests)
