set(CMAKE_BUILD_TYPE "Debug")
set(Qt5_DIR
    "C:/Qt/5.14.1/msvc2017_64/lib/cmake/Qt5"
    CACHE STRING "Path to the qt5 installation")
set(MSVC_DIA_SDK_DIR
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/DIA SDK"
    CACHE PATH "The DIA SDK path" FORCE)

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
