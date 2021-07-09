cmake_minimum_required(VERSION 3.15)

project(libunwindstack CXX C ASM)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Filesystem REQUIRED)

add_library(libsystem_headers INTERFACE)
target_include_directories(libsystem_headers INTERFACE libsystem/include/)

add_library(libutils_headers INTERFACE)
target_include_directories(libutils_headers INTERFACE libutils/include/)
target_link_libraries(libutils_headers INTERFACE libsystem_headers)

add_library(libcutils_headers INTERFACE)
target_include_directories(libcutils_headers INTERFACE libcutils/include/)

add_library(libbase_headers INTERFACE)
target_include_directories(libbase_headers INTERFACE libbase/include/)

add_library(liblog_headers INTERFACE)
target_include_directories(liblog_headers INTERFACE liblog/include/)

add_library(libprocinfo_headers INTERFACE)
target_include_directories(libprocinfo_headers INTERFACE libprocinfo/include/)


# libbase
file(GLOB libbase_source_files libbase/*.cpp)
list(FILTER libbase_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")
list(FILTER libbase_source_files EXCLUDE REGEX "(test_main|test_utils)\.cpp$")
if(WIN32)
  list(FILTER libbase_source_files EXCLUDE REGEX "_unix\.cpp$")
else()
  list(FILTER libbase_source_files EXCLUDE REGEX "(utf8|_windows)\.cpp$")
endif()

add_library(libbase OBJECT)
target_sources(libbase PRIVATE ${libbase_source_files})
target_link_libraries(libbase PUBLIC liblog_headers libbase_headers)

# libprocinfo
add_library(libprocinfo OBJECT)
file(GLOB libprocinfo_source_files libprocinfo/*.cpp)         
list(FILTER libprocinfo_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")
target_sources(libprocinfo PRIVATE ${libprocinfo_source_files})
target_include_directories(libprocinfo PUBLIC libprocinfo/include/)
target_link_libraries(libprocinfo PUBLIC libbase_headers)


# libunwindstack_common
add_library(libunwindstack_common OBJECT)
target_include_directories(libunwindstack_common PUBLIC libunwindstack/include/)

file(GLOB libunwindstack_source_files libunwindstack/*.cpp)         
list(FILTER libunwindstack_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")
list(FILTER libunwindstack_source_files EXCLUDE REGEX "(LogAndroid|LogStdout|DexFiles?)\.cpp$")
target_sources(libunwindstack_common PRIVATE ${libunwindstack_source_files})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
  target_sources(libunwindstack_common PRIVATE libunwindstack/AsmGetRegsX86_64.S)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86)
  target_sources(libunwindstack_common PRIVATE libunwindstack/AsmGetRegsX86.S)
endif()

target_link_libraries(
  libunwindstack_common
  PRIVATE libbase_headers
          libcutils_headers
          liblog_headers
          libprocinfo_headers
          libutils_headers
          CONAN_PKG::lzma_sdk)

# liblog
add_library(liblog_common OBJECT)
file(GLOB liblog_source_files liblog/*.cpp)
# These excluded files are android-only.
list(FILTER liblog_source_files EXCLUDE REGEX "(pmsg|logd)_(reader|writer)\.cpp$")
list(FILTER liblog_source_files EXCLUDE REGEX "(event_tag_map|log_time)\.cpp$")
target_sources(liblog_common PRIVATE ${liblog_source_files})
target_include_directories(liblog_common PUBLIC liblog/include/)
target_link_libraries(liblog_common PUBLIC libbase_headers libcutils_headers)


# libunwindstack
add_library(libunwindstack STATIC
  libunwindstack/LogStdout.cpp
  $<TARGET_OBJECTS:libunwindstack_common>
  $<TARGET_OBJECTS:liblog_common>)

target_include_directories(libunwindstack PUBLIC libunwindstack/include/)
target_link_libraries(libunwindstack PUBLIC libbase libprocinfo CONAN_PKG::lzma_sdk)
set_target_properties(libunwindstack PROPERTIES OUTPUT_NAME unwindstack)


### Unit Tests

# liblog_shared
add_library(liblog_shared SHARED $<TARGET_OBJECTS:liblog_common>)

enable_testing()
file(GLOB libunwindstack_test_files libunwindstack/tests/*.cpp libunwindstack/utils/*.cpp)         

# Both aren't test files
list(FILTER libunwindstack_test_files EXCLUDE REGEX "(TestLocal|GenGnuDebugdata)\.cpp$")

# We compile libunwindstack without dex-support - the following tests require dex-support:
list(FILTER libunwindstack_test_files EXCLUDE REGEX "(UnwinderTest|DexFiles?Test)\.cpp$")

# MemoryMte tests are only supported on Arch64
list(FILTER libunwindstack_test_files EXCLUDE REGEX "MemoryMteTest\.cpp$")

if(WIN32)
  # LocalUnwinderTest is only support on Linux as it uses `dlopen`/`dlsym`/etc.
  list(FILTER libunwindstack_test_files EXCLUDE REGEX "LocalUnwinderTest\.cpp$")
endif()

# The tests mock Android's logging system to capture its output. So we have to compile the tests with Android-Logging enabled,
# although the published library will use stdout logging which libunwindstack also supports.
add_executable(libunwindstack_tests libunwindstack/LogAndroid.cpp ${libunwindstack_test_files})

# We need to link against a shared version of liblog to avoid some linker errors. `FakeLog.cpp` redefines
# some symbols from liblog to intercept log messages. By linking dynamically we ensure that the tests use
# the symbols from `FakeLog.cpp` instead of the ones from liblog. We still need to link against liblog
# because `FakeLog.cpp` does not define all symbols from liblog.
target_link_libraries(
  libunwindstack_tests
  PUBLIC libbase
         libunwindstack_common
         liblog_shared
         std::filesystem
         CONAN_PKG::gtest)

if (NOT WIN32)
  # Needed by `LocalUnwinderTest.cpp`
  target_link_libraries(libunwindstack_tests PUBLIC dl)
endif()

# Tests need access to private headers
target_include_directories(
  libunwindstack_tests
  PRIVATE libunwindstack/
          libunwindstack/utils/)

# Some tests unwind their own callstacks so we have to disable inlining to make
# sure all the expected functions show up in the examined callstacks.
if (WIN32)
  target_compile_options(libunwindstack_tests PRIVATE /Od)
else()
  target_compile_options(libunwindstack_tests PRIVATE -O0)
endif()

add_test(NAME libunwindstack_tests COMMAND libunwindstack_tests)

# Copy test files to the expected location
file(INSTALL libunwindstack/offline_files DESTINATION bin)
file(INSTALL libunwindstack/tests/files DESTINATION bin/tests)