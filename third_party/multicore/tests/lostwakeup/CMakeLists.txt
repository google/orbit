cmake_minimum_required(VERSION 2.8.6)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE INTERNAL "limited configs")
project(LostWakeup)

set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mycompany.\${PRODUCT_NAME:identifier}")
file(GLOB FILES *.cpp *.h)
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${FILES})
include(../../cmake/BuildSettings.cmake)

add_subdirectory(../../common common)
include_directories(../../common)
target_link_libraries(${PROJECT_NAME} Common)
