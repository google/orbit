# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

# The setup is a little complex here. The libunwindstack tests need a lot of
# Android-infrastructure that we don't want to leak into OrbitService while
# libunwindstack supports being compiled without these dependencies.
# So we create a `libunwindstack_common` target which contains all the compile
# units that can be shared between the library and the tests and further
# down in the file we create a library target and a test target which consume
# the `_common` target.

add_library(libunwindstack_common OBJECT)
target_sources(libunwindstack_common PRIVATE
  AndroidUnwinder.cpp
  ArmExidx.cpp
  ArmExidx.h
  Check.h

  # We don't need DEX support
  # DexFile.cpp
  # DexFile.h

  # This file is needed regardless of DEX support
  DexFiles.cpp

  DwarfCfa.cpp
  DwarfCfa.h
  DwarfDebugFrame.h
  DwarfEhFrame.h
  DwarfEhFrameWithHdr.cpp
  DwarfEhFrameWithHdr.h
  DwarfEncoding.h
  DwarfMemory.cpp
  DwarfOp.cpp
  DwarfOp.h
  DwarfSection.cpp
  Elf.cpp
  ElfInterfaceArm.cpp
  ElfInterfaceArm.h
  ElfInterface.cpp
  Global.cpp
  GlobalDebugImpl.h
  JitDebug.cpp

  # LogAndroid is needed in the tests, while the lib needs LogStdout.
  # So we compile the `_common` target without both.
  # LogAndroid.cpp
  # LogStdout.cpp

  MapInfo.cpp
  Maps.cpp
  MemoryBuffer.h
  MemoryCache.h
  Memory.cpp
  MemoryFileAtOffset.h
  MemoryLocal.h
  MemoryMte.cpp
  MemoryOfflineBuffer.h
  MemoryOffline.h
  MemoryRange.h
  MemoryRemote.h
  MemoryXz.cpp
  MemoryXz.h
  Object.cpp
  PeCoff.cpp
  PeCoffEpilog.cpp
  PeCoffEpilog.h
  PeCoffInterface.cpp
  PeCoffRuntimeFunctions.cpp
  PeCoffRuntimeFunctions.h
  PeCoffUnwindInfoEvaluator.h
  PeCoffUnwindInfoEvaluator.cpp
  PeCoffUnwindInfos.cpp
  PeCoffUnwindInfos.h
  PeCoffUnwindInfoUnwinderX86_64.h
  PeCoffUnwindInfoUnwinderX86_64.cpp
  RegsArm64.cpp
  RegsArm.cpp
  Regs.cpp
  RegsInfo.h
  RegsMips64.cpp
  RegsMips.cpp
  RegsX86_64.cpp
  RegsX86.cpp
  Symbols.cpp
  Symbols.h
  ThreadEntry.cpp
  ThreadEntry.h
  ThreadUnwinder.cpp
  Unwinder.cpp)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
  target_sources(libunwindstack_common PRIVATE AsmGetRegsX86_64.S)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86)
  target_sources(libunwindstack_common PRIVATE AsmGetRegsX86.S)
endif()

target_sources(libunwindstack_common PUBLIC
  include/GlobalDebugInterface.h
  include/unwindstack/AndroidUnwinder.h
  include/unwindstack/Arch.h
  include/unwindstack/DexFiles.h
  include/unwindstack/DwarfError.h
  include/unwindstack/DwarfLocation.h
  include/unwindstack/DwarfMemory.h
  include/unwindstack/DwarfSection.h
  include/unwindstack/DwarfStructs.h
  include/unwindstack/Elf.h
  include/unwindstack/ElfInterface.h
  include/unwindstack/Error.h
  include/unwindstack/Global.h
  include/unwindstack/JitDebug.h
  include/unwindstack/Log.h
  include/unwindstack/MachineArm.h
  include/unwindstack/MachineArm64.h
  include/unwindstack/MachineMips.h
  include/unwindstack/MachineMips64.h
  include/unwindstack/MachineX86.h
  include/unwindstack/MachineX86_64.h
  include/unwindstack/MapInfo.h
  include/unwindstack/Maps.h
  include/unwindstack/Memory.h
  include/unwindstack/Object.h
  include/unwindstack/PeCoff.h
  include/unwindstack/PeCoffInterface.h
  include/unwindstack/PeCoffNativeUnwinder.h
  include/unwindstack/Regs.h
  include/unwindstack/RegsArm.h
  include/unwindstack/RegsArm64.h
  include/unwindstack/RegsGetLocal.h
  include/unwindstack/RegsMips.h
  include/unwindstack/RegsMips64.h
  include/unwindstack/RegsX86.h
  include/unwindstack/RegsX86_64.h
  include/unwindstack/SharedString.h
  include/unwindstack/UcontextArm.h
  include/unwindstack/UcontextArm64.h
  include/unwindstack/UcontextMips.h
  include/unwindstack/UcontextMips64.h
  include/unwindstack/UcontextX86.h
  include/unwindstack/UcontextX86_64.h
  include/unwindstack/Unwinder.h
  include/unwindstack/UserArm.h
  include/unwindstack/UserArm64.h
  include/unwindstack/UserMips.h
  include/unwindstack/UserMips64.h
  include/unwindstack/UserX86.h
  include/unwindstack/UserX86_64.h)

target_include_directories(libunwindstack_common PUBLIC include/)
target_compile_features(libunwindstack_common PUBLIC cxx_std_17)
target_link_libraries(libunwindstack_common PRIVATE
  capstone::capstone
  libbase
  libprocinfo
  LZMA::LZMA)

# Clang >= 8.0
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
  target_compile_options(libunwindstack_common PRIVATE
    -Wno-error=ctad-maybe-unsupported)
endif()

target_compile_options(libunwindstack_common PRIVATE
  -Wno-error=return-type
  -Wno-error=sign-compare
  -Wno-error=format-zero-length
  -Wno-error=format-nonliteral)

# libunwindstack library - to be consumed by LinuxTracing and others.
add_library(libunwindstack STATIC
  $<TARGET_OBJECTS:libunwindstack_common>
  LogStdout.cpp)

target_link_libraries(libunwindstack PUBLIC
  capstone::capstone
  libbase
  libprocinfo

  # liblog_static needs to be listed AFTER libbase and libprocinfo due to cyclic depdendencies.
  liblog_static
  LZMA::LZMA)
target_include_directories(libunwindstack PUBLIC include/)

# Clang >= 8.0
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
  target_compile_options(libunwindstack PRIVATE
    -Wno-error=ctad-maybe-unsupported)
endif()

target_compile_options(libunwindstack PRIVATE
    -Wno-error=format-nonliteral)

# add_executable(libunwindstack_tests)
# target_sources(libunwindstack_tests PRIVATE
#   $<TARGET_OBJECTS:libunwindstack_common>
#   LogAndroid.cpp

#   # AndroidUnwinderTest are broken and need to be investigated,
#   # but we currently don't use this part of the library.
#   # tests/AndroidUnwinderTest.cpp

#   tests/ArmExidxDecodeTest.cpp
#   tests/ArmExidxExtractTest.cpp

#   # We compile without DEX file support
#   # tests/DexFileTest.cpp
#   # tests/DexFilesTest.cpp

#   tests/DwarfCfaLogTest.cpp
#   tests/DwarfCfaTest.cpp
#   tests/DwarfDebugFrameTest.cpp
#   tests/DwarfEhFrameTest.cpp
#   tests/DwarfEhFrameWithHdrTest.cpp
#   tests/DwarfMemoryTest.cpp
#   tests/DwarfOpLogTest.cpp
#   tests/DwarfOpTest.cpp
#   tests/DwarfSectionImplTest.cpp
#   tests/DwarfSectionTest.cpp
#   tests/ElfFake.cpp
#   tests/ElfInterfaceArmTest.cpp
#   tests/ElfInterfaceTest.cpp
#   tests/ElfTest.cpp
#   tests/ElfTestUtils.cpp

#   # Not a test file
#   # tests/GenGnuDebugdata.cpp

#   tests/GlobalDebugImplTest.cpp
#   tests/GlobalTest.cpp
#   tests/IsolatedSettings.cpp
#   tests/JitDebugTest.cpp
#   tests/LocalUpdatableMapsTest.cpp
#   tests/LogFake.cpp
#   tests/MapInfoCreateMemoryTest.cpp
#   tests/MapInfoGetBuildIDTest.cpp
#   tests/MapInfoGetLoadBiasTest.cpp
#   tests/MapInfoGetObjectTest.cpp
#   tests/MapInfoTest.cpp
#   tests/MapsTest.cpp
#   tests/MemoryBufferTest.cpp
#   tests/MemoryCacheTest.cpp
#   tests/MemoryFileTest.cpp
#   tests/MemoryLocalTest.cpp

#   # This is Android-specific
#   # tests/MemoryMteTest.cpp

#   tests/MemoryOfflineBufferTest.cpp
#   tests/MemoryOfflineTest.cpp
#   tests/MemoryRangeTest.cpp
#   tests/MemoryRangesTest.cpp
#   tests/MemoryRemoteTest.cpp
#   tests/MemoryTest.cpp
#   tests/MemoryThreadCacheTest.cpp
#   tests/MemoryXzTest.cpp
#   tests/ObjectBuildIdTest.cpp
#   tests/ObjectCacheTest.cpp
#   tests/PeCoffInterfaceTest.cpp
#   tests/PeCoffFake.cpp
#   tests/PeCoffRuntimeFunctionsTest.cpp
#   tests/PeCoffTest.cpp
#   tests/PeCoffUnwindInfoEvaluatorTest.cpp
#   tests/PeCoffUnwindInfosTest.cpp
#   tests/PeCoffUnwindInfoUnwinderX86_64Test.cpp
#   tests/PeCoffEpilogTest.cpp
#   tests/RegsInfoTest.cpp
#   tests/RegsIterateTest.cpp
#   tests/RegsStepIfSignalHandlerTest.cpp
#   tests/RegsRemoteTest.cpp
#   tests/RegsTest.cpp
#   tests/SymbolsTest.cpp

#   # TestLocal is not a test. It belongs to a testing library.
#   # tests/TestLocal.cpp

#   tests/TestUtils.cpp
#   tests/UnwindOfflineTest.cpp
#   tests/UnwindTest.cpp
#   tests/UnwinderTest.cpp
#   tests/VerifyBionicTerminationTest.cpp
#   utils/MemoryFake.cpp
#   utils/OfflineUnwindUtils.cpp
#   utils/PidUtils.cpp
#   utils/ProcessTracer.cpp)

# # Some tests call dlopen/dlsym/dlclose which requires the
# # dynamic linker.
# target_link_libraries(libunwindstack_tests PRIVATE ${CMAKE_DL_LIBS})

# target_link_libraries(libunwindstack_tests PRIVATE
#   liblog_shared
#   libbase
#   libprocinfo
#   capstone::capstone
#   LZMA::LZMA
#   GTest::Main
#   ZLIB::ZLIB)

# target_include_directories(libunwindstack_tests PRIVATE
#   ${CMAKE_CURRENT_SOURCE_DIR}
#   utils/
#   include/)

# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#   target_compile_options(libunwindstack_tests PRIVATE
#     -Wno-error=format-nonliteral
#     -Wno-error=inconsistent-missing-override
#     -Wno-error=unused-result
#     -Wno-error=defaulted-function-deleted)
# else()
#   target_compile_options(libunwindstack_tests PRIVATE
#     -Wno-error=format-nonliteral
#     -Wno-error=unused-result)
# endif()

# # Some tests unwind its own callstacks so we have to disable inlining to make
# # sure all the expected functions show up in the examined callstacks.
# # They also invoke a lot of different kinds of UB which would all be optimized away,
# # if we didn't disable optimization.
# target_compile_options(libunwindstack_tests PRIVATE -O0)

# register_test(libunwindstack_tests)

# # The tests expect the testdata next to the test binary.
# add_custom_command(TARGET libunwindstack_tests POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E remove_directory
#     $<TARGET_FILE_DIR:libunwindstack_tests>/offline_files
#   COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_CURRENT_LIST_DIR}/offline_files
#     $<TARGET_FILE_DIR:libunwindstack_tests>/offline_files)

# add_custom_command(TARGET libunwindstack_tests POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E remove_directory
#     $<TARGET_FILE_DIR:libunwindstack_tests>/tests
#   COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_CURRENT_LIST_DIR}/tests
#     $<TARGET_FILE_DIR:libunwindstack_tests>/tests)



# # This is a testing library that will be consumed by the tests.
# add_library(libunwindstack_local SHARED
#   $<TARGET_OBJECTS:libunwindstack_common>
#   LogStdout.cpp
#   tests/TestLocal.cpp)

# target_link_libraries(libunwindstack_local PRIVATE
#   libbase

#   # liblog_static needs to be listed AFTER libbase due to cyclic depdendencies.
#   liblog_static
#   LZMA::LZMA)

# target_include_directories(libunwindstack_local PRIVATE include/)

# # Clang >= 8.0
# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
#     CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
#   target_compile_options(libunwindstack_local PRIVATE
#     -Wno-error=ctad-maybe-unsupported)
# endif()

# target_compile_options(libunwindstack_local PRIVATE
#   -Wno-error=format-nonliteral)

# # This library unwinds its own callstacks so we have to disable inlining
# # to make sure all the expected functions show up in the examined callstacks.
# # It also invokes a lot of different kinds of UB which would all be optimized away,
# # if we didn't disable optimization.
# target_compile_options(libunwindstack_local PRIVATE -O0)

# set_target_properties(libunwindstack_local PROPERTIES OUTPUT_NAME "unwindstack_local")

# # The tests expect this library next to the executable.
# set_target_properties(libunwindstack_local PROPERTIES
#          LIBRARY_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:libunwindstack_tests>)
# set_target_properties(libunwindstack_local PROPERTIES
#          LIBRARY_OUTPUT_DIRECTORY_RELEASE $<TARGET_FILE_DIR:libunwindstack_tests>)
# set_target_properties(libunwindstack_local PROPERTIES
#          LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO $<TARGET_FILE_DIR:libunwindstack_tests>)
# set_target_properties(libunwindstack_local PROPERTIES
#          LIBRARY_OUTPUT_DIRECTORY_DEBUG $<TARGET_FILE_DIR:libunwindstack_tests>)
