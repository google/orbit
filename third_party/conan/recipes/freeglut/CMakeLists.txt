cmake_minimum_required(VERSION 2.8.12)
project(cmake_wrapper)

include(conanbuildinfo.cmake)
conan_basic_setup()

add_definitions("-DFREEGLUT_LIB_PRAGMAS=0")

add_subdirectory("source_subfolder")
