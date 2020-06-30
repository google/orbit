# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitFuzzing)

add_executable(OrbitFuzzerElf elfFuzzer.cpp)
target_compile_options(OrbitFuzzerElf PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(OrbitFuzzerElf PRIVATE OrbitCore)
target_compile_options(OrbitFuzzerElf PUBLIC "-fsanitize=fuzzer")
set_target_properties(OrbitFuzzerElf PROPERTIES LINK_OPTIONS "-fsanitize=fuzzer")

add_executable(OrbitFuzzerLoadCapture loadCaptureFuzzer.cpp)
target_compile_options(OrbitFuzzerLoadCapture PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(OrbitFuzzerLoadCapture PRIVATE OrbitGl)
target_compile_options(OrbitFuzzerLoadCapture PUBLIC "-fsanitize=fuzzer")
set_target_properties(OrbitFuzzerLoadCapture PROPERTIES LINK_OPTIONS "-fsanitize=fuzzer")

add_executable(OrbitFuzzerTcpServer TcpServerFuzzer.cpp)
target_compile_options(OrbitFuzzerTcpServer PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(OrbitFuzzerTcpServer PRIVATE OrbitServiceLib)
target_compile_options(OrbitFuzzerTcpServer PUBLIC "-fsanitize=fuzzer")
set_target_properties(OrbitFuzzerTcpServer PROPERTIES LINK_OPTIONS "-fsanitize=fuzzer")

add_executable(OrbitFuzzerTcpClient TcpClientFuzzer.cpp)
target_compile_options(OrbitFuzzerTcpClient PRIVATE ${STRICT_COMPILE_FLAGS})
target_link_libraries(OrbitFuzzerTcpClient PRIVATE OrbitGl)
target_compile_options(OrbitFuzzerTcpClient PUBLIC "-fsanitize=fuzzer")
set_target_properties(OrbitFuzzerTcpClient PROPERTIES LINK_OPTIONS "-fsanitize=fuzzer")