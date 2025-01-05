# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Orbit C CXX ASM)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Werror=all
                      -Werror=abstract-final-class
                      -Werror=float-conversion
                      -Werror=format=2
                      -Werror=ignored-attributes
                      -Werror=implicit-fallthrough
                      -Werror=inconsistent-missing-override
                      -Werror=old-style-cast
                      -Werror=unused-parameter
                      -Werror=unused-variable
                      -Werror=writable-strings
                      -Werror=sign-compare
                      -Werror=thread-safety)

  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
    add_compile_options(-Werror=defaulted-function-deleted
                        # Required by Google-internal builds
                        -Werror=ctad-maybe-unsupported)
  endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Werror=all
                      # gcc does not behave consistently here; what is fine with gcc9 fails with gcc10
                      # and vice versa, see https://github.com/google/orbit/issues/1624 for details.
                      -Wno-stringop-truncation
                      -Werror=float-conversion
                      -Werror=format=2
                      -Werror=ignored-attributes
                      -Werror=old-style-cast
                      -Werror=unused-parameter
                      -Werror=unused-variable
                      -Werror=sign-compare
                      # These seem to be buggy in GCC 11 and 12:
                      -Wno-maybe-uninitialized
                      -Wno-uninitialized
                      -Wno-stringop-overflow)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_GUI "Setting this option will enable the Qt-based UI client." ON)

#if(NOT WIN32)
#  option(WITH_VULKAN "Setting this option will enable Vulkan" ON)

#  if(WITH_VULKAN)
#    add_definitions(-DVULKAN_ENABLED)
#  endif()
#endif()

# This is only for designated initializers
if(WIN32)
  set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ Standard" FORCE)
else()
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/cmake")

if (NOT WIN32)
  # Make all executables position independent on Linux

  # Note that we need to have both. If we omit "-pie"
  # option executables are built as ET_EXEC, if we
  # omit set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  # Ninja configuration stops working because for
  # some reason in config stage it compile code for tests
  # without -fPIC but tries to link them with -pie

  string(APPEND CMAKE_EXE_LINKER_FLAGS " -pie")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()


find_package(Threads REQUIRED)
find_package(xxHash REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(gte REQUIRED)
#find_package(libprotobuf-mutator REQUIRED)
find_package(LZMA REQUIRED)
find_package(absl REQUIRED)
find_package(LLVM REQUIRED)
find_package(gRPC REQUIRED)
find_package(capstone REQUIRED)
find_package(protobuf REQUIRED)
find_package(outcome REQUIRED)
find_package(ZLIB REQUIRED)

if(WITH_GUI)
  find_package(OpenGL REQUIRED)
  find_package(
    Qt5 CONFIG REQUIRED
    COMPONENTS Core
               Network
               Test
               Widgets)
  find_package(Libssh2 REQUIRED)
endif()

find_package(Filesystem REQUIRED)

if(WIN32)
  find_package(DiaSdk REQUIRED)
  find_package(krabsetw REQUIRED)
  find_package(PresentMon REQUIRED)
else()
  find_package(libbase REQUIRED)
  find_package(liblog REQUIRED)
  find_package(libprocinfo REQUIRED)
  if(WITH_VULKAN)
    find_package(volk REQUIRED)
    find_package(VulkanHeaders REQUIRED)
    find_package(vulkan-validationlayers REQUIRED)
  endif()
endif()

include("cmake/protobuf.cmake")
include("cmake/grpc_helper.cmake")
#include("cmake/fuzzing.cmake")
include("cmake/tests.cmake")
include("cmake/iwyu.cmake")
enable_testing()

# Set preprocessor defines These are only necessary for windows, but we will
# define them on all platforms, to keep the builds similar as possible.
add_definitions(-DNOMINMAX)
add_definitions(-DUNICODE -D_UNICODE)

if (MSVC)
  # Build with Multiple Processes.
  add_compile_options("/MP")
  # Set source and execution character sets to UTF-8.
  add_compile_options("/utf-8")
  # Set warning level for external includes.
  string(APPEND CMAKE_CXX_FLAGS " /experimental:external /external:anglebrackets /external:W0")
  string(APPEND CMAKE_C_FLAGS " /experimental:external /external:anglebrackets /external:W0")
  # Treat all compiler warnings as errors.
  add_compile_options("/WX")
endif()

if(WIN32)
  add_definitions(-DWIN32)
  add_subdirectory(src/WindowsCaptureService)
  add_subdirectory(src/WindowsProcessLauncherService)
  add_subdirectory(src/WindowsProcessService)
  add_subdirectory(src/WindowsTracing)
  add_subdirectory(src/WindowsUtils)
  add_subdirectory(third_party/minhook)
else()
  add_subdirectory(src/LinuxCaptureService)
  add_subdirectory(src/FakeClient)
  add_subdirectory(src/LinuxTracing)
  add_subdirectory(src/LinuxTracingIntegrationTests)
  add_subdirectory(src/MemoryTracing)
  add_subdirectory(src/OrbitClientGgp)
  add_subdirectory(src/OrbitCaptureGgpClient)
  add_subdirectory(src/OrbitCaptureGgpService)
  add_subdirectory(src/ProcessService)
  add_subdirectory(src/TracepointService)
  add_subdirectory(src/UserSpaceInstrumentation)
  add_subdirectory(third_party/libunwindstack)
  if(WITH_VULKAN)
    add_subdirectory(src/OrbitTriggerCaptureVulkanLayer)
    add_subdirectory(src/OrbitVulkanLayer)
    add_subdirectory(src/VulkanTutorial)
  endif()
endif()

add_subdirectory(src/Api)
add_subdirectory(src/ApiInterface)
add_subdirectory(src/ApiLoader)
add_subdirectory(src/ApiUtils)
add_subdirectory(src/CaptureClient)
add_subdirectory(src/CaptureEventProducer)
add_subdirectory(src/CaptureFile)
add_subdirectory(src/CaptureServiceBase)
add_subdirectory(src/ClientData)
add_subdirectory(src/ClientFlags)
add_subdirectory(src/ClientModel)
add_subdirectory(src/ClientProtos)
add_subdirectory(src/ClientServices)
add_subdirectory(src/Containers)
add_subdirectory(src/CrashService)
add_subdirectory(src/DisplayFormats)
add_subdirectory(src/FakeProducerSideService)
#add_subdirectory(src/FuzzingUtils)
add_subdirectory(src/GrpcProtos)
add_subdirectory(src/Introspection)
add_subdirectory(src/ModuleUtils)
add_subdirectory(src/ObjectUtils)
add_subdirectory(src/OrbitAccessibility)
add_subdirectory(src/OrbitBase)
add_subdirectory(src/OrbitPaths)
add_subdirectory(src/OrbitTest)
add_subdirectory(src/OrbitVersion)
add_subdirectory(src/PresetFile)
add_subdirectory(src/ProducerEventProcessor)
add_subdirectory(src/ProducerSideChannel)
add_subdirectory(src/ProducerSideService)
add_subdirectory(src/Service)
add_subdirectory(src/StringManager)
add_subdirectory(src/SymbolProvider)
add_subdirectory(src/Symbols)
add_subdirectory(src/Statistics)
add_subdirectory(src/Test)
add_subdirectory(src/TestUtils)

if(WITH_GUI)
  add_subdirectory(src/CaptureFileInfo)
  add_subdirectory(src/ClientSymbols)
  add_subdirectory(src/CodeReport)
  add_subdirectory(src/CodeViewer)
  add_subdirectory(src/CodeViewerDemo)
  add_subdirectory(src/ConfigWidgets)
  add_subdirectory(src/CommandLineUtils)
  add_subdirectory(src/DataViews)
  add_subdirectory(src/Http)
  add_subdirectory(src/Mizar)
  add_subdirectory(src/MizarBase)
  add_subdirectory(src/MizarData)
  add_subdirectory(src/MizarModels)
  add_subdirectory(src/MizarWidgets)
  add_subdirectory(src/MizarStatistics)
  add_subdirectory(src/Orbit)
  add_subdirectory(src/OrbitGl)
  add_subdirectory(src/OrbitSsh)
  add_subdirectory(src/OrbitSshQt)
  add_subdirectory(src/OrbitQt)
  add_subdirectory(src/QtTestUtils)
  add_subdirectory(src/QtUtils)
  add_subdirectory(src/RemoteSymbolProvider)
  add_subdirectory(src/SessionSetup)
  add_subdirectory(src/SourcePathsMapping)
  add_subdirectory(src/SourcePathsMappingDialogDemo)
  add_subdirectory(src/SourcePathsMappingUI)
  add_subdirectory(src/SshQtTestUtils)
  add_subdirectory(src/Style)
  add_subdirectory(src/SyntaxHighlighter)
  add_subdirectory(src/UtilWidgets)
endif()

if(WIN32)
  # Startup Project
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                              "Orbit")
endif()

# Collecting third-party licenses
file(COPY third_party/concurrentqueue/LICENSE.md DESTINATION ${CMAKE_BINARY_DIR}/licenses/concurrentqueue/)
file(COPY third_party/xxHash-r42/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/xxhash/)
file(COPY third_party/gte/LICENSE_1_0.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/gte/)
file(COPY third_party/Qt/LICENSE.LGPLv3 DESTINATION ${CMAKE_BINARY_DIR}/licenses/Qt/)
file(COPY third_party/Outcome/LICENCE DESTINATION ${CMAKE_BINARY_DIR}/licenses/Outcome/licenses/)
file(COPY third_party/capstone/LICENSE.TXT DESTINATION ${CMAKE_BINARY_DIR}/licenses/capstone/licenses/)
file(COPY third_party/LLVM/LICENSE.TXT DESTINATION ${CMAKE_BINARY_DIR}/licenses/LLVM/licenses/)
file(COPY third_party/lzma_sdk/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/lzma_sdk/licenses/)
file(COPY third_party/conan/LICENSE.md DESTINATION ${CMAKE_BINARY_DIR}/licenses/conan/licenses/)
file(COPY third_party/vulkan/LICENSE.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/vulkan/licenses/)
file(COPY third_party/krabsetw/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/krabsetw/)
file(COPY "third_party/DIA SDK/LICENSE" DESTINATION "${CMAKE_BINARY_DIR}/licenses/DIA SDK/")
file(COPY third_party/cppwin32/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/cppwin32/)
file(COPY third_party/cppwin32/cppwin32/winmd/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/winmd/)
file(COPY third_party/minhook/LICENSE.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/minhook/)
file(COPY third_party/PresentMon/LICENSE.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/PresentMon/)


# Generating license NOTICE files
include("cmake/collect_licenses.cmake")
GenerateThirdPartyLicenseFile("${CMAKE_BINARY_DIR}/NOTICE" "${CMAKE_BINARY_DIR}/licenses/")
