set(CMAKE_BUILD_TYPE
    Debug
    CACHE STRING "build type" FORCE)
set(CMAKE_C_COMPILER clang-9)
set(CMAKE_CXX_COMPILER clang++-9)
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE BOOL "generate compile_commands.json" FORCE)

string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld")
string(APPEND CMAKE_MODULE_LINKER_FLAGS " -fuse-ld=lld")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " -fuse-ld=lld")

add_compile_options(
  -Wpedantic
  -Wall
  -Wextra
  -Wdouble-promotion
  -Wformat=2
  -Wnull-dereference
  -Wimplicit-fallthrough
  -Wmissing-include-dirs
  -Wshift-overflow
  -Wswitch-enum
  -Wunused-parameter
  -Wunused-const-variable
  -Wunknown-pragmas
  -Warray-bounds
  -Wfloat-equal
  -Wundef
  -Wunused-macros
  -Wcast-qual
  -Wcast-align
  -Wconversion
  -Wdate-time
  -Wsign-conversion
  -Wmissing-declarations
  -Wpacked
  -Wredundant-decls
  -Winvalid-pch
  -Wdisabled-optimization
  -Wstack-protector
  -fcolor-diagnostics
  -fno-omit-frame-pointer)
