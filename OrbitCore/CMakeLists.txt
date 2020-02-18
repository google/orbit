cmake_minimum_required(VERSION 3.15)

project(OrbitCore)
add_library(OrbitCore STATIC)

set(HEADERS include/OrbitCore)

target_sources(
  OrbitCore
  PUBLIC ${HEADERS}/BaseTypes.h
         ${HEADERS}/BpfTrace.h
         ${HEADERS}/BlockChain.h
         ${HEADERS}/Callstack.h
         ${HEADERS}/CallstackTypes.h
         ${HEADERS}/Capture.h
         ${HEADERS}/Context.h
         ${HEADERS}/ContextSwitch.h
         ${HEADERS}/ConnectionManager.h
         ${HEADERS}/Core.h
         ${HEADERS}/CoreApp.h
         ${HEADERS}/CrashHandler.h
         ${HEADERS}/Diff.h
         ${HEADERS}/EventBuffer.h
         ${HEADERS}/EventClasses.h
         ${HEADERS}/FunctionStats.h
         ${HEADERS}/Hashing.h
         ${HEADERS}/Injection.h
         ${HEADERS}/LinuxCallstackEvent.h
         ${HEADERS}/LinuxPerf.h
         ${HEADERS}/Log.h
         ${HEADERS}/LogInterface.h
         ${HEADERS}/MemoryTracker.h
         ${HEADERS}/Message.h
         ${HEADERS}/MiniDump.h
         ${HEADERS}/ModuleManager.h
         ${HEADERS}/ModuleManager.h
         ${HEADERS}/OrbitAsio.h
         ${HEADERS}/OrbitDbgHelp.h
         ${HEADERS}/OrbitFunction.h
         ${HEADERS}/OrbitLib.h
         ${HEADERS}/OrbitModule.h
         ${HEADERS}/OrbitProcess.h
         ${HEADERS}/OrbitRule.h
         ${HEADERS}/OrbitSession.h
         ${HEADERS}/OrbitThread.h
         ${HEADERS}/OrbitType.h
         ${HEADERS}/OrbitUnreal.h
         ${HEADERS}/OrbitUserData.h
         ${HEADERS}/Params.h
         ${HEADERS}/Path.h
         ${HEADERS}/Pdb.h
         ${HEADERS}/PrintVar.h
         ${HEADERS}/ProcessUtils.h
         ${HEADERS}/Profiling.h
         ${HEADERS}/RingBuffer.h
         ${HEADERS}/SamplingProfiler.h
         ${HEADERS}/ScopeTimer.h
         ${HEADERS}/Serialization.h
         ${HEADERS}/SerializationMacros.h
         ${HEADERS}/Systrace.h
         ${HEADERS}/Tcp.h
         ${HEADERS}/TcpClient.h
         ${HEADERS}/TcpEntity.h
         ${HEADERS}/TcpServer.h
         ${HEADERS}/TcpForward.h
         ${HEADERS}/TestRemoteMessages.h
         ${HEADERS}/Threading.h
         ${HEADERS}/TimerManager.h
         ${HEADERS}/TypeInfoStructs.h
         ${HEADERS}/Utils.h
         ${HEADERS}/Variable.h
         ${HEADERS}/VariableTracing.h
         ${HEADERS}/Version.h)

target_sources(
  OrbitCore
  PRIVATE BpfTrace.cpp
          Callstack.cpp
          Capture.cpp
          ContextSwitch.cpp
          Core.cpp
          CoreApp.cpp
          CrashHandler.cpp
          ConnectionManager.cpp
          Diff.cpp
          EventBuffer.cpp
          FunctionStats.cpp
          Injection.cpp
          LinuxCallstackEvent.cpp
          LinuxPerf.cpp
          Log.cpp
          LogInterface.cpp
          MemoryTracker.cpp
          Message.cpp
          ModuleManager.cpp
          MiniDump.cpp
          ModuleManager.cpp
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
          ProcessUtils.cpp
          Profiling.cpp
          SamplingProfiler.cpp
          ScopeTimer.cpp
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
          VariableTracing.cpp
          Version.cpp)

if(WIN32)
  target_sources(
    OrbitCore
    PUBLIC ${HEADERS}/DiaManager.h
           ${HEADERS}/DiaParser.h
           ${HEADERS}/EventCallbacks.h
           ${HEADERS}/EventGuid.h
           ${HEADERS}/EventTracer.h
           ${HEADERS}/EventUtils.h
           ${HEADERS}/Hijacking.h
           ${HEADERS}/OrbitDia.h
           ${HEADERS}/SymbolUtils.h)

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
    PUBLIC ${HEADERS}/BpfTraceVisitor.h
           ${HEADERS}/BpfTrace.h
           ${HEADERS}/LibunwindstackUnwinder.h
           ${HEADERS}/LinuxEventTracer.h
           ${HEADERS}/LinuxPerfEvent.h
           ${HEADERS}/LinuxPerfEventProcessor.h
           ${HEADERS}/LinuxPerfEventVisitor.h
           ${HEADERS}/LinuxPerfRingBuffer.h
           ${HEADERS}/LinuxPerfUtils.h
           ${HEADERS}/LinuxUprobesUnwindingVisitor.h
           ${HEADERS}/LinuxUtils.h)

  target_sources(
    OrbitCore
    PRIVATE BpfTraceVisitor.cpp
            LibunwindstackUnwinder.cpp
            LinuxEventTracer.cpp
            LinuxUtils.cpp
            LinuxPerfEvent.cpp
            LinuxPerfEventProcessor.cpp
            LinuxPerfRingBuffer.cpp
            LinuxPerfUtils.cpp
            LinuxUprobesUnwindingVisitor.cpp)
endif()

target_include_directories(OrbitCore PUBLIC include/)
target_include_directories(OrbitCore PRIVATE include/OrbitCore/)

target_link_libraries(
  OrbitCore
  PUBLIC cereal
         xxHash::xxHash
         concurrentqueue::concurrentqueue
         multicore::multicore
         CURL::libcurl
         oqpi::oqpi
         asio::asio
         absl::str_format)

if(WIN32)
  target_link_libraries(
    OrbitCore
    PRIVATE minhook::minhook DIA2Dump::DIA2Dump
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client OrbitAsm)
  target_link_libraries(OrbitCore PRIVATE dbghelp.lib peparse::peparse)
  target_compile_definitions(OrbitCore PUBLIC -D_WIN32_WINNT=0x0700)
  target_compile_definitions(OrbitCore PUBLIC -DNTDDI_VERSION=0x06030000)
else()
  target_link_libraries(OrbitCore PUBLIC unwindstack::unwindstack)
endif()

target_compile_definitions(OrbitCore PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(OrbitCore PUBLIC INITGUID)
target_compile_definitions(OrbitCore PUBLIC NOMINMAX)
target_compile_definitions(OrbitCore PUBLIC UNICODE)
target_compile_definitions(OrbitCore PUBLIC _UNICODE)

target_compile_features(OrbitCore PUBLIC cxx_std_11)

add_executable(OrbitCoreTests)

target_sources(OrbitCoreTests PRIVATE RingBufferTests.cpp)

if(NOT WIN32)
  target_sources(OrbitCoreTests PRIVATE LinuxUprobesUnwindingVisitorTests.cpp)
endif()

target_link_libraries(OrbitCoreTests PRIVATE OrbitCore GTest::GTest GTest::Main)
