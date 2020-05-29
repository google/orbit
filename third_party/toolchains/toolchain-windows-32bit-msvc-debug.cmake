set(CMAKE_BUILD_TYPE
    "Debug"
    CACHE STRING "build type" FORCE)

add_compile_options(
  /W4
  /wd4100
  /wd4245
  /wd4244
  /wd4481
  /wd4201
  /Zi)

string(APPEND CMAKE_CXX_FLAGS " /MP")

add_link_options(/INCREMENTAL:NO)

