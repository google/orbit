set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckStructHasMember)

function(PrintVars var)
message("=== PRINT VARS ===")
message("=")
message(${var})
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    if(_variableName MATCHES ${var})
        message(STATUS "${_variableName}=${${_variableName}}")
    endif()
endforeach()
message("=")
message("=== PRINT VARS ===")
endfunction()

function(PrintAllVars)
message("=== ALL VARS ===")
message("=")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message("=")
message("=== ALL VARS ===")
endfunction(PrintAllVars)

set(EXTERN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../external")
#get_filename_component(EXTERN_ROOT ${CMAKE_SOURCE_DIR}/external REALPATH)

set(EXTERNAL_HEADERS

#    ${EXTERN_ROOT}/DIA2Dump/Callback.h
#    ${EXTERN_ROOT}/DIA2Dump/dia2dump.h
#    ${EXTERN_ROOT}/DIA2Dump/PrintSymbol.h
#    ${EXTERN_ROOT}/DIA2Dump/regs.h
#    ${EXTERN_ROOT}/peparse/nt-headers.h
#    ${EXTERN_ROOT}/peparse/parse.h
#    ${EXTERN_ROOT}/peparse/to_string.h

    ${EXTERN_ROOT}/multicore/common/autoresetevent.h
    ${EXTERN_ROOT}/multicore/common/autoreseteventcondvar.h
    ${EXTERN_ROOT}/multicore/common/benaphore.h
    ${EXTERN_ROOT}/multicore/common/bitfield.h
    ${EXTERN_ROOT}/multicore/common/diningphilosophers.h
    ${EXTERN_ROOT}/multicore/common/inmemorylogger.h
    ${EXTERN_ROOT}/multicore/common/rwlock.h
    ${EXTERN_ROOT}/multicore/common/sema.h
)

set(EXTERNAL_SOURCES

#    ${EXTERN_ROOT}/DIA2Dump/dia2dump.cpp
#    ${EXTERN_ROOT}/DIA2Dump/PrintSymbol.cpp
#    ${EXTERN_ROOT}/DIA2Dump/regs.cpp
#    ${EXTERN_ROOT}/peparse/buffer.cpp
#    ${EXTERN_ROOT}/peparse/dump.cpp
#    ${EXTERN_ROOT}/peparse/parse.cpp
    #${EXTERN_ROOT}/multicore/common/inmemorylogger.cpp

    ${EXTERN_ROOT}/xxHash-r42/xxhash.c
)

source_group("External\\Header Files" FILES ${EXTERNAL_HEADERS})
source_group("External\\Source Files" FILES ${EXTERNAL_SOURCES})

#PrintAllVars()

#pkg_check_modules(GLEW glew REQUIRED)
find_package(glew REQUIRED)
PrintVars(glew)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(cereal REQUIRED)
include_directories("${_VCPKG_INSTALLED_DIR}/x64-linux/include")
PrintVars(cereal)

add_definitions(
    -Wfatal-errors
    )

set(HEADERS
    BaseTypes.h
    BlockChain.h
    Callstack.h
    CallstackTypes.h
    Capture.h
    Context.h
    ContextSwitch.h
    Core.h
    CoreApp.h
	CrashHandler.h
    Diff.h
    EventBuffer.h
    EventClasses.h
    FunctionStats.h
    Hashing.h
    Injection.h
	Log.h
    LogInterface.h
    MemoryTracker.h
	Message.h
    MiniDump.h
    ModuleManager.h
	OrbitAsio.h
    OrbitDbgHelp.h
    OrbitFunction.h
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
    Tcp.h
    TcpClient.h
    TcpEntity.h
    TcpServer.h
	TcpForward.h
	Threading.h
    TimerManager.h
	TypeInfoStructs.h
    Utils.h
    Variable.h
    VariableTracing.h
    Version.h
)

set(PLATFORM_HEADERS
    DiaManager.h
    DiaParser.h
    EventCallbacks.h
    EventGuid.h
    EventTracer.h
    EventUtils.h
    Hijacking.h
    OrbitDia.h
    OrbitLib.h
    SymbolUtils.h
)

set(SOURCES
    Callstack.cpp
    Capture.cpp
    ContextSwitch.cpp
    Core.cpp
    CoreApp.cpp
	CrashHandler.cpp
    Diff.cpp
    EventBuffer.cpp
    FunctionStats.cpp
    Injection.cpp
	Log.cpp
    LogInterface.cpp
    MemoryTracker.cpp
	Message.cpp
    MiniDump.cpp
    ModuleManager.cpp
	OrbitAsio.cpp
    OrbitFunction.cpp
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
    Tcp.cpp
    Tcp.cpp
    TcpClient.cpp
    TcpEntity.cpp
    TcpServer.cpp
    TimerManager.cpp
    #TypeInfo.cpp
    Variable.cpp
    VariableTracing.cpp
    Version.cpp
)

set(PLATFORM_SOURCES
    DiaManager.cpp
    DiaParser.cpp
    EventCallbacks.cpp
    EventTracer.cpp
    EventUtils.cpp
    Hijacking.cpp
    OrbitDia.cpp
    OrbitLib.cpp
    Pdb.cpp
    SymbolUtils.cpp
    Utils.cpp
    TypeInfo.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitCore STATIC
    ${SOURCES} ${EXTERNAL_SOURCES}
    ${HEADERS} ${EXTERNAL_HEADERS}
)

target_include_directories(OrbitCore
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEREAL_INCLUDE_DIR}
    ${EXTERN_ROOT}/concurrentqueue
    ${EXTERN_ROOT}/multicore/common/
    ${EXTERN_ROOT}/xxHash-r42
#    ${EXTERN_ROOT}/oqpi/include
    ${EXTERN_ROOT}/breakpad/src/
    ${EXTERN_ROOT}/minhook/include/

#    ${EXTERN_ROOT}/DIA2Dump/
#    ${DIASDKDir}/include

PRIVATE
    ${BREAKPAD_INCLUDE_DIR}
)

target_link_libraries(OrbitCore
#    dbghelp.lib
    ${BREAKPAD_LIBRARIES}
)
