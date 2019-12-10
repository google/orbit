cmake_minimum_required(VERSION 3.6)

# Orbit directories
set(ORBIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(EXTERN_ROOT "${ORBIT_ROOT}/external")
set(ORBIT_UTILS "${ORBIT_ROOT}/OrbitUtils")

# Orbit utils (PrintVars, PrintAllVars...)
include(${ORBIT_UTILS}/utils.cmake)

# Vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${EXTERN_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# OrbitCore
project(OrbitCore)

# Headers
set(HEADERS
    BaseTypes.h
    BpfTrace.h
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
    Diff.h
    EventBuffer.h
    EventClasses.h
    FunctionStats.h
    Hashing.h
    Injection.h
    LinuxPerf.h
    LinuxPerfData.h
    Log.h
    LogInterface.h
    MemoryTracker.h
    Message.h
    MiniDump.h
    ModuleManager.h
    ModuleManager.h
    OrbitAsio.h
    OrbitDbgHelp.h
    OrbitFunction.h
    OrbitLib.h
    OrbitModule.h
    OrbitProcess.h
    OrbitRule.h
    OrbitSession.h
    OrbitThread.h
    OrbitType.h
    OrbitUnreal.h
    Params.h
    Path.h
    Pdb.h
    PrintVar.h
    ProcessUtils.h
    Profiling.h
    RingBuffer.h
    SamplingProfiler.h
    ScopeTimer.h
    Serialization.h
    SerializationMacros.h
    Systrace.h
    Tcp.h
    TcpClient.h
    TcpEntity.h
    TcpServer.h
    TcpForward.h
    TestRemoteMessages.h
    Threading.h
    TimerManager.h
    TypeInfoStructs.h
    Utils.h
    Variable.h
    VariableTracing.h
    Version.h
)

# Sources
set(SOURCES
    BpfTrace.cpp
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
    LinuxPerf.cpp
    LinuxPerfData.cpp
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
    OrbitRule.cpp
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
    Version.cpp
    ${EXTERN_ROOT}/xxHash-r42/xxhash.c
)

# Windows
if(WIN32)
    set(PLATFORM_HEADERS
        DiaManager.h
        DiaParser.h
        EventCallbacks.h
        EventGuid.h
        EventTracer.h
        EventUtils.h
        Hijacking.h
        OrbitDia.h
        SymbolUtils.h
    )
    set(PLATFORM_SOURCES
        DiaManager.cpp
        DiaParser.cpp
        EventCallbacks.cpp
        EventTracer.cpp
        EventUtils.cpp
        Hijacking.cpp
        OrbitDia.cpp
        Pdb.cpp
        SymbolUtils.cpp
        Utils.cpp
		${EXTERN_ROOT}/xxHash-r42/xxhash.c
    )
    set(EXTERNAL_HEADERS
        ${EXTERN_ROOT}/DIA2Dump/Callback.h
        ${EXTERN_ROOT}/DIA2Dump/dia2dump.h
        ${EXTERN_ROOT}/DIA2Dump/PrintSymbol.h
        ${EXTERN_ROOT}/DIA2Dump/regs.h
        ${EXTERN_ROOT}/peparse/nt-headers.h
        ${EXTERN_ROOT}/peparse/parse.h
        ${EXTERN_ROOT}/peparse/to_string.h
        ${EXTERN_ROOT}/xxHash-r42/xxhash.h
        ${EXTERN_ROOT}/multicore/common/autoresetevent.h)

	set(EXTERNAL_SOURCES
    ${EXTERN_ROOT}/DIA2Dump/dia2dump.cpp
    ${EXTERN_ROOT}/DIA2Dump/PrintSymbol.cpp
    ${EXTERN_ROOT}/DIA2Dump/regs.cpp
    ${EXTERN_ROOT}/multicore/common/inmemorylogger.cpp
    ${EXTERN_ROOT}/peparse/buffer.cpp
    ${EXTERN_ROOT}/peparse/dump.cpp
    ${EXTERN_ROOT}/peparse/parse.cpp
    ${EXTERN_ROOT}/xxHash-r42/xxhash.c
)
endif()

# Linux
if(LINUX)
    set(PLATFORM_HEADERS 
        BpfTraceVisitor.cpp
        LinuxUtils.h    
        LinuxPerfEvent.h
        LinuxPerfEventVisitor.h
        LinuxPerfEventProcessor.h
        LinuxPerfRingBuffer.h
        LinuxPerfUtils.h
        LinuxEventTracer.h
        LinuxEventTracerVisitor.h
    )
    set(PLATFORM_SOURCES 
        BpfTraceVisitor.cpp
        LinuxUtils.cpp
        LinuxPerfEvent.cpp
        LinuxPerfRingBuffer.cpp
        LinuxPerfUtils.cpp
        LinuxEventTracer.cpp
        LinuxEventTracerVisitor.cpp
        LinuxPerfEventProcessor.cpp
    )
	set(EXTERNAL_SOURCES "")
    set(EXTERNAL_HEADERS
        ${EXTERN_ROOT}/xxHash-r42/xxhash.h
        ${EXTERN_ROOT}/multicore/common/autoresetevent.h)
endif()

# Groups
source_group("Header Files" FILES ${HEADERS} ${PLATFORM_HEADERS})
source_group("Source Files" FILES ${SOURCES} ${PLATFORM_SOURCES})
source_group("External\\Header Files" FILES ${EXTERNAL_HEADERS})
source_group("External\\Source Files" FILES ${EXTERNAL_SOURCES})

# Find cereal
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(cereal REQUIRED)

# Stop compilation on first error
if(LINUX)
include_directories("${_VCPKG_INSTALLED_DIR}/x64-linux/include")
add_definitions(-Wfatal-errors)
endif()

if(WIN32)
#So that #include "processor/linked_ptr.h" works in core_modules.h
include_directories("${_VCPKG_INSTALLED_DIR}/x64-windows/include/google_breakpad")
endif()

# Library
add_library(OrbitCore STATIC
    ${SOURCES} ${PLATFORM_SOURCES} ${EXTERNAL_SOURCES}
    ${HEADERS} ${PLATFORM_HEADERS} ${EXTERNAL_HEADERS}
)

# Include directories
target_include_directories(OrbitCore
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEREAL_INCLUDE_DIR}
    ${EXTERN_ROOT}/concurrentqueue/
    ${EXTERN_ROOT}/multicore/common/
    ${EXTERN_ROOT}/xxHash-r42/
    ${EXTERN_ROOT}/minhook/include/
    ${EXTERN_ROOT}/oqpi/include/
    ${EXTERN_ROOT}/DIA2Dump/
PRIVATE
    ${BREAKPAD_INCLUDE_DIR}
)

set(DIASDKDir "$(VSInstallDir)DIA SDK" CACHE PATH "")
add_definitions(-I"${DIASDKDir}/include/")

# Link libraries
target_link_libraries(OrbitCore
    #dbghelp.lib
    #${BREAKPAD_LIBRARIES}
)
