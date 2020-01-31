set(CMAKE_BUILD_TYPE "Debug")
set(MSVC_DIA_SDK_DIR
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/DIA SDK"
    CACHE PATH "The DIA SDK path" FORCE)

add_compile_options(
  /W4
  /wd4100
  /wd4245
  /wd4244
  /wd4481
  /wd4201
  /Zi)

add_link_options(/INCREMENTAL:NO)

# This include expects your build directory to be a direct subdirectory of the
# project root.
include(
  ${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg/scripts/buildsystems/vcpkg.cmake)
