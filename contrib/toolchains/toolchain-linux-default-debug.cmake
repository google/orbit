set(CMAKE_BUILD_TYPE
    Debug
    CACHE STRING "build type" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    CACHE BOOL "generate compile_commands.json" FORCE)

add_compile_options(
  -Wpedantic
  -Wall
  -Wextra
  -Wdouble-promotion
  -Wformat=2
  -Wnull-dereference
#  -Wimplicit-fallthrough  # This option is not supported on gamelets.
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
  -fno-omit-frame-pointer)
