set(CMAKE_BUILD_TYPE
    "Release"
    CACHE STRING "build type")

set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE BOOL "generate compile_commands.json" FORCE)

set(VCPKG_TARGET_TRIPLET
    "x64-linux-ggp"
    CACHE STRING "vcpkg's target triplet" FORCE)
