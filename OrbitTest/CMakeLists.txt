cmake_minimum_required(VERSION 3.6)

# Orbit directories
set(ORBIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(EXTERN_ROOT "${ORBIT_ROOT}/external")
set(ORBIT_UTILS "${ORBIT_ROOT}/OrbitUtils")

# Orbit utils (PrintVars, PrintAllVars...)
include(${ORBIT_UTILS}/utils.cmake)

# Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# OrbitTest
set(MSVC_INCREMENTAL_DEFAULT ON)
project(OrbitTest)


# Stop compilation on first error
if(LINUX)
add_definitions(-Wfatal-errors)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
    OrbitTest.h
)
set(SOURCES
    main.cpp
    OrbitTest.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(OrbitTest
    ${SOURCES} ${HEADERS}
)

target_include_directories(OrbitTest
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(OrbitTest PROPERTIES OUTPUT_NAME "OrbitTest")
