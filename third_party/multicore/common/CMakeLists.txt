project(Common)

file(GLOB FILES *.h *.cpp)
add_library(Common ${FILES})
include(../cmake/BuildSettings.cmake)
