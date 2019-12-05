cmake_minimum_required(VERSION 3.6)

set(ORBIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(EXTERN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(ANDROID_CORE_ROOT "${EXTERN_ROOT}/libunwindstack/android-core")
set(UNWINDSTACK_ROOT "${ANDROID_CORE_ROOT}/libunwindstack")
set(ANDROID_BASE_ROOT "${ANDROID_CORE_ROOT}/base")
set(LZMA_ROOT "${EXTERN_ROOT}/lzma")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas -Wno-sign-compare")

project(unwindstack)

include_directories(
    ${UNWINDSTACK_ROOT} 
    ${UNWINDSTACK_ROOT}/include
    ${ANDROID_BASE_ROOT}/include 
    ${ANDROID_CORE_ROOT}/include
    ${ANDROID_CORE_ROOT}/liblog/include
    ${ANDROID_CORE_ROOT}/libprocinfo/include
    ${ANDROID_CORE_ROOT}/demangle/include
    ${LZMA_ROOT}/lzma/C
)

# TODO: We might miss sources from "base" or "demangle" when linking
set(SOURCES
    #${ANDROID_BASE_ROOT}/file.cpp
    #${ANDROID_BASE_ROOT}/logging.cpp
    #${ANDROID_BASE_ROOT}/stringprintf.cpp
    #${ANDROID_BASE_ROOT}/strings.cpp
    #${ANDROID_BASE_ROOT}/threads.cpp
    ${UNWINDSTACK_ROOT}/ArmExidx.cpp
    ${UNWINDSTACK_ROOT}/DwarfCfa.cpp
    ${UNWINDSTACK_ROOT}/DwarfEhFrameWithHdr.cpp
    ${UNWINDSTACK_ROOT}/DwarfMemory.cpp
    ${UNWINDSTACK_ROOT}/DwarfOp.cpp
    ${UNWINDSTACK_ROOT}/DwarfSection.cpp
    ${UNWINDSTACK_ROOT}/Elf.cpp
    ${UNWINDSTACK_ROOT}/ElfInterface.cpp
    ${UNWINDSTACK_ROOT}/ElfInterfaceArm.cpp
    ${UNWINDSTACK_ROOT}/Global.cpp
    ${UNWINDSTACK_ROOT}/JitDebug.cpp
    ${UNWINDSTACK_ROOT}/LocalUnwinder.cpp
    ${UNWINDSTACK_ROOT}/Log.cpp
    ${UNWINDSTACK_ROOT}/MapInfo.cpp
    ${UNWINDSTACK_ROOT}/Maps.cpp
    ${UNWINDSTACK_ROOT}/Memory.cpp
    ${UNWINDSTACK_ROOT}/Regs.cpp
    ${UNWINDSTACK_ROOT}/RegsArm.cpp
    ${UNWINDSTACK_ROOT}/RegsArm64.cpp
    ${UNWINDSTACK_ROOT}/RegsMips.cpp
    ${UNWINDSTACK_ROOT}/RegsMips64.cpp
    ${UNWINDSTACK_ROOT}/RegsX86.cpp
    ${UNWINDSTACK_ROOT}/RegsX86_64.cpp
    ${UNWINDSTACK_ROOT}/Symbols.cpp
    ${UNWINDSTACK_ROOT}/Unwinder.cpp
    ${UNWINDSTACK_ROOT}/tests/LogFake.cpp
)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
    SET(ASM_GET_REGS ${UNWINDSTACK_ROOT}/AsmGetRegsX86_64.S)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86)
    SET(ASM_GET_REGS ${UNWINDSTACK_ROOT}/AsmGetRegsX86.S)
endif()


add_library(unwindstack STATIC ${SOURCES} ${ASM_GET_REGS})