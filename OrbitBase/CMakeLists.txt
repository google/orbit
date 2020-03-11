cmake_minimum_required(VERSION 3.15)

project(OrbitBase)

add_library(OrbitBase INTERFACE)

target_compile_features(OrbitBase INTERFACE cxx_std_17)

target_include_directories(OrbitBase INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitBase INTERFACE
        include/OrbitBase/Logging.h)

target_link_libraries(OrbitBase INTERFACE
        abseil::abseil
        std::filesystem)

if (false)  # TODO: Remove if (false) once we have at least one test.
add_executable(OrbitBaseTests)

target_sources(OrbitBaseTests PRIVATE
        )

target_link_libraries(OrbitBaseTests PRIVATE
        OrbitBase
        GTest::GTest
        GTest::Main)
endif()

add_test(NAME OrbitBase COMMAND OrbitBaseTests)
