# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitCore)
add_library(OrbitCore STATIC)

target_compile_options(OrbitCore PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitCore
  PUBLIC BaseTypes.h
         BlockChain.h
         Callstack.h
         CallstackTypes.h
         Capture.h
         Context.h
         ContextSwitch.h
         ConnectionManager.h
         Core.h
         CoreApp.h
         CrashHandler.h
         CrashOptions.h
         EventBuffer.h
         EventClasses.h
         FramePointerValidator.h
         FunctionFramePointerValidator.h
         FunctionStats.h
         Hashing.h
         Injection.h
         Introspection.h
         LinuxAddressInfo.h
         LinuxCallstackEvent.h
         LinuxTracingBuffer.h
         Log.h
         LogInterface.h
         MemoryTracker.h
         Message.h
         OrbitAsio.h
         OrbitDbgHelp.h
         OrbitFunction.h
         OrbitLib.h
         OrbitModule.h
         OrbitProcess.h
         OrbitSession.h
         OrbitThread.h
         OrbitType.h
         OrbitUnreal.h
         Params.h
         Path.h
         Pdb.h
         PrintVar.h
         ProcessMemoryRequest.h
         ProcessUtils.h
         Profiling.h
         RingBuffer.h
         SamplingProfiler.h
         ScopeTimer.h
         Serialization.h
         SerializationMacros.h
         StringManager.h
         SymbolHelper.h
         Systrace.h
         Tcp.h
         TcpClient.h
         TcpEntity.h
         TcpServer.h
         TcpForward.h
         TestRemoteMessages.h
         Threading.h
         TidAndThreadName.h
         TimerManager.h
         Utils.h
         Variable.h
         VariableTracing.h
         Version.h)

target_sources(
  OrbitCore
  PRIVATE Callstack.cpp
          Capture.cpp
          ContextSwitch.cpp
          Core.cpp
          CoreApp.cpp
          CrashHandler.cpp
          ConnectionManager.cpp
          ElfFile.cpp
          EventBuffer.cpp
          FramePointerValidator.cpp
          FunctionFramePointerValidator.cpp
          FunctionStats.cpp
          Injection.cpp
          Introspection.cpp
          LinuxTracingBuffer.cpp
          Log.cpp
          LogInterface.cpp
          MemoryTracker.cpp
          Message.cpp
          OrbitAsio.cpp
          OrbitFunction.cpp
          OrbitLib.cpp
          OrbitModule.cpp
          OrbitProcess.cpp
          OrbitSession.cpp
          OrbitThread.cpp
          OrbitType.cpp
          OrbitUnreal.cpp
          Params.cpp
          Path.cpp
          ProcessMemoryRequest.cpp
          ProcessUtils.cpp
          Profiling.cpp
          SamplingProfiler.cpp
          ScopeTimer.cpp
          StringManager.cpp
          SymbolHelper.cpp
          Systrace.cpp
          Tcp.cpp
          Tcp.cpp
          TcpClient.cpp
          TcpEntity.cpp
          TcpServer.cpp
          TestRemoteMessages.cpp
          TimerManager.cpp
          Utils.cpp
          Variable.cpp
          VariableTracing.cpp)

if(WIN32)
  target_sources(
    OrbitCore
    PUBLIC DiaManager.h
           DiaParser.h
           EventCallbacks.h
           EventGuid.h
           EventTracer.h
           EventUtils.h
           Hijacking.h
           OrbitDia.h
           SymbolUtils.h)

  target_sources(
    OrbitCore
    PRIVATE DiaManager.cpp
            DiaParser.cpp
            EventCallbacks.cpp
            EventTracer.cpp
            EventUtils.cpp
            Hijacking.cpp
            OrbitDia.cpp
            Pdb.cpp
            SymbolUtils.cpp
            Utils.cpp)
else()
  target_sources(
    OrbitCore
    PUBLIC LinuxTracingHandler.h
           LinuxUtils.h)

  target_sources(
    OrbitCore
    PRIVATE LinuxTracingHandler.cpp
            LinuxUtils.cpp)
endif()

target_include_directories(OrbitCore PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
  OrbitCore
  PUBLIC OrbitBase
         capstone::capstone
         cereal::cereal
         xxHash::xxHash
         concurrentqueue::concurrentqueue
         multicore::multicore
         oqpi::oqpi
         asio::asio
         abseil::abseil
         llvm_object::llvm_object
         crashpad::crashpad
         Outcome::Outcome)

if(WIN32)
  target_link_libraries(
    OrbitCore
    PRIVATE minhook::minhook DIA2Dump::DIA2Dump
            OrbitAsm)
  target_link_libraries(OrbitCore PRIVATE dbghelp.lib peparse::peparse)
  target_compile_definitions(OrbitCore PUBLIC -D_WIN32_WINNT=0x0700)
  target_compile_definitions(OrbitCore PUBLIC -DNTDDI_VERSION=0x06030000)
else()
  target_link_libraries(
    OrbitCore
    PUBLIC OrbitLinuxTracing
           libunwindstack::libunwindstack)
endif()

target_compile_definitions(OrbitCore PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(OrbitCore PUBLIC INITGUID)
target_compile_definitions(OrbitCore PUBLIC NOMINMAX)
target_compile_definitions(OrbitCore PUBLIC UNICODE)
target_compile_definitions(OrbitCore PUBLIC _UNICODE)

target_compile_features(OrbitCore PUBLIC cxx_std_11)

include("${CMAKE_SOURCE_DIR}/cmake/version.cmake")
GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/Version.cpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in" OrbitCore)
target_sources(OrbitCore PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CrashOptions.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/CrashOptions.cpp)
target_sources(OrbitCore PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/CrashOptions.cpp")

add_executable(OrbitCoreTests)

target_sources(OrbitCoreTests PRIVATE
    ElfFileTest.cpp
    FunctionFramepointerValidatorTest.cpp
    FramePointerValidatorTest.cpp
    LinuxTracingBufferTest.cpp
    PathTest.cpp
    RingBufferTest.cpp
    StringManagerTest.cpp
    SymbolHelperTest.cpp
)

if(NOT WIN32)
  target_sources(OrbitCoreTests PRIVATE OrbitModuleTest.cpp)
endif()

target_link_libraries(
  OrbitCoreTests
  PRIVATE OrbitCore
          GTest::Main
          GMock::GMock
          llvm_object::llvm_object
          abseil::abseil)

add_custom_command(TARGET OrbitCoreTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/testdata
  $<TARGET_FILE_DIR:OrbitCoreTests>/testdata)

register_test(OrbitCoreTests)
