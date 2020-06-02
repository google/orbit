set(CMAKE_BUILD_TYPE
    Release
    CACHE STRING "build type" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE BOOL "generate compile_commands.json" FORCE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

string(APPEND CMAKE_CXX_FLAGS " -march=skylake")
