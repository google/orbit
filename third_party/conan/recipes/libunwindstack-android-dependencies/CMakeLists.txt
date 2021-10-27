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
list(FILTER libbase_source_files EXCLUDE REGEX "(utf8|_windows)\.cpp$")

add_library(libbase STATIC)
target_sources(libbase PRIVATE ${libbase_source_files})
target_link_libraries(libbase PUBLIC liblog_headers libbase_headers)

# The "lib" prefix will be added automatically.
set_target_properties(libbase PROPERTIES OUTPUT_NAME "base")

# libprocinfo
add_library(libprocinfo STATIC)
file(GLOB libprocinfo_source_files libprocinfo/*.cpp)         
list(FILTER libprocinfo_source_files EXCLUDE REGEX "_(test|fuzzer|benchmark)\.cpp$")
target_sources(libprocinfo PRIVATE ${libprocinfo_source_files})
target_include_directories(libprocinfo PUBLIC libprocinfo/include/)
target_link_libraries(libprocinfo PUBLIC libbase_headers)

# The "lib" prefix will be added automatically.
set_target_properties(libprocinfo PROPERTIES OUTPUT_NAME "procinfo")

# liblog
add_library(liblog_common OBJECT)
file(GLOB liblog_source_files liblog/*.cpp)
# These excluded files are android-only.
list(FILTER liblog_source_files EXCLUDE REGEX "(pmsg|logd)_(reader|writer)\.cpp$")
list(FILTER liblog_source_files EXCLUDE REGEX "(event_tag_map|log_time)\.cpp$")
target_sources(liblog_common PRIVATE ${liblog_source_files} libbase/logging.cpp)
target_include_directories(liblog_common PUBLIC liblog/include/)
target_link_libraries(liblog_common PUBLIC libbase_headers libcutils_headers)

add_library(log_shared SHARED $<TARGET_OBJECTS:liblog_common>)
add_library(log_static STATIC $<TARGET_OBJECTS:liblog_common>)