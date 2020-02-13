set(CMAKE_BUILD_TYPE "Debug")
include(${CMAKE_CURRENT_LIST_DIR}/find_dia_sdk_path.cmake)
find_dia_sdk_path("2017")

string(APPEND CMAKE_CXX_FLAGS " /W4")
string(APPEND CMAKE_CXX_FLAGS " /wd4100")
string(APPEND CMAKE_CXX_FLAGS " /wd4245")
string(APPEND CMAKE_CXX_FLAGS " /wd4244")
string(APPEND CMAKE_CXX_FLAGS " /wd4481")
string(APPEND CMAKE_CXX_FLAGS " /wd4201")
string(APPEND CMAKE_CXX_FLAGS " /EHsc")
string(APPEND CMAKE_CXX_FLAGS " /MP")
string(APPEND CMAKE_C_FLAGS " /MP")

# This include expects your build directory to be a direct subdirectory of the
# project root.
include(
  ${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg/scripts/buildsystems/vcpkg.cmake)
