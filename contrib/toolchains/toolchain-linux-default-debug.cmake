set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
  -Wstack-protector)

include(
  "${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg/scripts/buildsystems/vcpkg.cmake"
)
