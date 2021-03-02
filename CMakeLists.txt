# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Orbit C CXX)

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
  set(STRICT_COMPILE_FLAGS -Werror=all
                           -Werror=float-conversion
                           -Werror=format=2
                           -Werror=ignored-attributes
                           -Werror=implicit-fallthrough
                           -Werror=inconsistent-missing-override
                           -Werror=old-style-cast
                           -Werror=unused-parameter
                           -Werror=unused-variable
                           -Werror=writable-strings
                           -Werror=sign-compare)

  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
    list(APPEND STRICT_COMPILE_FLAGS -Werror=defaulted-function-deleted)
  endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(STRICT_COMPILE_FLAGS -Werror=all
                           -Werror=float-conversion
                           -Werror=format=2
                           -Werror=ignored-attributes
                           -Werror=old-style-cast
                           -Werror=unused-parameter
                           -Werror=unused-variable
                           -Werror=sign-compare)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_GUI "Setting this option will enable the Qt-based UI client." ON)
option(WITH_CRASH_HANDLING "Setting this option will enable crash handling based on crashpad." ON)
option(WITH_VULKAN_LAYER "Setting this option will enable the Vulkan layer for advanced GPU information." OFF)

set(CRASHDUMP_SERVER "" CACHE STRING "Setting this option will enable uploading crash dumps to the url specified.")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

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
find_package(multicore REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(gte REQUIRED)
find_package(oqpi REQUIRED)

if(WITH_GUI)
  find_package(OpenGL REQUIRED)
  find_package(
    Qt5 CONFIG REQUIRED
    COMPONENTS Core
               Network
               Test
               Widgets
               WebEngine
               WebEngineWidgets
               WebSockets
               WebChannel)
  find_package(qtpropertybrowser REQUIRED)
  find_package(stb REQUIRED)
endif()

find_package(Filesystem REQUIRED)

if(NOT WIN32 AND WITH_VULKAN_LAYER)
  find_package(Vulkan REQUIRED)
  find_package(VulkanValidationLayers REQUIRED)
endif()

include("cmake/protobuf.cmake")
include("cmake/grpc_helper.cmake")
include("cmake/fuzzing.cmake")
include("cmake/strip.cmake")
include("cmake/tests.cmake")
include("cmake/iwyu.cmake")
enable_testing()

# Set preprocessor defines These are only necessary for windows, but we will
# define them on all platforms, to keep the builds similar as possible.
add_definitions(-DNOMINMAX)
add_definitions(-DUNICODE -D_UNICODE)

if(WIN32)
  add_definitions(-DWIN32)
else()
  add_subdirectory(src/Api)
  add_subdirectory(src/LinuxTracing)
  add_subdirectory(src/MemoryTracing)
  add_subdirectory(src/OrbitAPI)
  add_subdirectory(src/OrbitClientGgp)
  add_subdirectory(src/OrbitCaptureGgpClient)
  add_subdirectory(src/OrbitCaptureGgpService)
  add_subdirectory(src/OrbitProducer)
  add_subdirectory(src/ProducerSideChannel)
  add_subdirectory(src/Service)
  add_subdirectory(src/UserSpaceInstrumentation)
endif()

if(WITH_VULKAN_LAYER)
  #add_subdirectory(src/OrbitTriggerCaptureVulkanLayer)
  #add_subdirectory(src/OrbitVulkanLayer)
endif()

add_subdirectory(src/ClientProtos)
add_subdirectory(src/ElfUtils)
add_subdirectory(src/FramePointerValidator)
add_subdirectory(src/GrpcProtos)
add_subdirectory(src/MetricsUploader)
add_subdirectory(src/OrbitAccessibility)
add_subdirectory(src/OrbitBase)
add_subdirectory(src/OrbitCaptureClient)
add_subdirectory(src/OrbitClientData)
add_subdirectory(src/OrbitClientModel)
add_subdirectory(src/OrbitClientServices)
add_subdirectory(src/OrbitCore)
add_subdirectory(src/OrbitTest)
add_subdirectory(src/OrbitVersion)

if(WITH_GUI)
  add_subdirectory(src/CodeViewer)
  add_subdirectory(src/CodeViewerDemo)
  add_subdirectory(src/ConfigWidgets)
  add_subdirectory(src/OrbitGgp)
  add_subdirectory(src/OrbitGl)
  add_subdirectory(src/OrbitSsh)
  add_subdirectory(src/OrbitSshQt)
  add_subdirectory(src/OrbitQt)
  add_subdirectory(src/QtUtils)
  add_subdirectory(src/SyntaxHighlighter)
  add_subdirectory(src/SourcePathsMapping)
  add_subdirectory(src/SourcePathsMappingDialogDemo)
  add_subdirectory(src/WebEngine)
  add_subdirectory(src/WebUI)
endif()

if(WITH_GUI AND WITH_CRASH_HANDLING)
  add_subdirectory(src/CrashHandler)
endif()

if(WIN32)
  # Startup Project
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                              "OrbitQt")
endif()

# Collecting third-party licenses
file(COPY third_party/concurrentqueue/LICENSE.md DESTINATION ${CMAKE_BINARY_DIR}/licenses/concurrentqueue/)
file(COPY third_party/multicore/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/multicore/)
file(COPY third_party/xxHash-r42/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/xxhash/)
file(COPY third_party/gte/LICENSE_1_0.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/gte/)
file(COPY third_party/oqpi/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/oqpi/)
file(COPY third_party/Qt/LICENSE.LGPLv3 DESTINATION ${CMAKE_BINARY_DIR}/licenses/Qt/)
file(COPY third_party/Outcome/LICENCE DESTINATION ${CMAKE_BINARY_DIR}/licenses/Outcome/licenses/)
file(COPY third_party/capstone/LICENSE.TXT DESTINATION ${CMAKE_BINARY_DIR}/licenses/capstone/licenses/)
file(COPY third_party/LLVM/LICENSE.TXT DESTINATION ${CMAKE_BINARY_DIR}/licenses/LLVM/licenses/)
file(COPY third_party/libunwindstack/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/libunwindstack/licenses/)
file(COPY third_party/lzma_sdk/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/lzma_sdk/licenses/)
file(COPY third_party/freetype-gl/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/freetype-gl/licenses/)
file(COPY third_party/fonts/VERA-COPYRIGHT DESTINATION ${CMAKE_BINARY_DIR}/licenses/fonts/)
file(COPY third_party/OpenSSL/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/openssl/licenses/)
file(COPY third_party/qtpropertybrowser/LICENSE.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/qtpropertybrowser/licenses/)
file(COPY third_party/conan/LICENSE.md DESTINATION ${CMAKE_BINARY_DIR}/licenses/conan/licenses/)
file(COPY third_party/stb/LICENSE DESTINATION ${CMAKE_BINARY_DIR}/licenses/stb/licenses/)
file(COPY third_party/vulkan/LICENSE.txt DESTINATION ${CMAKE_BINARY_DIR}/licenses/vulkan/licenses/)
file(REMOVE ${CMAKE_BINARY_DIR}/licenses/freetype/licenses/LICENSE.TXT)
file(REMOVE ${CMAKE_BINARY_DIR}/licenses/freetype/licenses/GPLv2.TXT)


# Generating license NOTICE files
include("cmake/collect_licenses.cmake")

# When not compiling with a system-provided version of Qt we can integrate all the licenses from Chromium.
if(EXISTS "${CMAKE_BINARY_DIR}/NOTICE.Chromium")
  GenerateThirdPartyLicenseFile("${CMAKE_BINARY_DIR}/NOTICE" "${CMAKE_BINARY_DIR}/licenses/" APPEND_FILES "${CMAKE_BINARY_DIR}/NOTICE.Chromium")
else()
  GenerateThirdPartyLicenseFile("${CMAKE_BINARY_DIR}/NOTICE" "${CMAKE_BINARY_DIR}/licenses/")
endif()
