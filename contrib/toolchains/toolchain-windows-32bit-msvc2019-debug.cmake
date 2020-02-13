set(CMAKE_BUILD_TYPE "Debug")
include(${CMAKE_CURRENT_LIST_DIR}/find_dia_sdk_path.cmake)
find_dia_sdk_path("2019")

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
