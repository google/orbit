cmake_minimum_required(VERSION 3.15)

project(unwindstack CXX C ASM)

add_library(unwindstack STATIC)

set(ANDROID_CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/android-core")
set(UNWINDSTACK_ROOT "${ANDROID_CORE_ROOT}/libunwindstack")
set(ANDROID_BASE_ROOT "${ANDROID_CORE_ROOT}/base")

# Contains Android's or libunwindstack's files that we want to replace because
# they cause building issues on gamelets or that would cause too many additional
# Android dependencies.
target_include_directories(unwindstack PUBLIC overrides/)

# set_source_files_properties(ASM_GET_REGS
# ${UNWINDSTACK_ROOT}/AsmGetRegsX86_64.S PROPERTIES COMPILE_FLAGS "-x assembler-
# with-cpp") set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

target_include_directories(
  unwindstack SYSTEM
  PUBLIC ${UNWINDSTACK_ROOT}/include
         ${ANDROID_BASE_ROOT}/include
         ${ANDROID_CORE_ROOT}/include
         # ${ANDROID_CORE_ROOT}/demangle/include
         ${ANDROID_CORE_ROOT}/liblog/include
         ${ANDROID_CORE_ROOT}/libprocinfo/include)

target_include_directories(unwindstack PRIVATE ${UNWINDSTACK_ROOT})

# Override file.cpp to avoid depending on Android's logging. Override
# strings.h/.cpp to not use <string_view>, which requires C++17 that gamelets'
# GCC does not support. Override libunwindstack's files that gamelet's GCC fails
# to compile. TODO: Until we switch to clang, make sure to update the overrides
# of libunwindstack's files if the originals are updated.
target_sources(
  unwindstack
  PRIVATE # ${ANDROID_BASE_ROOT}/file.cpp
          overrides/file.cpp
          # ${ANDROID_BASE_ROOT}/liblog_symbols.cpp
          # ${ANDROID_BASE_ROOT}/logging.cpp
          ${ANDROID_BASE_ROOT}/stringprintf.cpp
          # ${ANDROID_BASE_ROOT}/strings.cpp
          overrides/strings.cpp
          ${ANDROID_BASE_ROOT}/threads.cpp
          # ${ANDROID_CORE_ROOT}/liblog/properties.cpp
          ${UNWINDSTACK_ROOT}/ArmExidx.cpp
          ${UNWINDSTACK_ROOT}/DwarfCfa.cpp
          ${UNWINDSTACK_ROOT}/DwarfEhFrameWithHdr.cpp
          ${UNWINDSTACK_ROOT}/DwarfMemory.cpp
          ${UNWINDSTACK_ROOT}/DwarfOp.cpp
          # ${UNWINDSTACK_ROOT}/DwarfSection.cpp
          overrides/DwarfSection.cpp
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
          ${UNWINDSTACK_ROOT}/tests/LogFake.cpp)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
  target_sources(unwindstack PRIVATE ${UNWINDSTACK_ROOT}/AsmGetRegsX86_64.S)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86)
  target_sources(unwindstack PRIVATE ${UNWINDSTACK_ROOT}/AsmGetRegsX86.S)
endif()

target_link_libraries(unwindstack PRIVATE lzma)
add_library(unwindstack::unwindstack ALIAS unwindstack)
