project(OrbitService)

add_executable(OrbitService)

target_compile_options(OrbitService PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitService PRIVATE main.cpp OrbitService.cpp)
target_sources(OrbitService PUBLIC OrbitService.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(OrbitService PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(OrbitService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitService PRIVATE OrbitCore)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # Administrator privileges
  set_target_properties(
    OrbitService
    PROPERTIES LINK_FLAGS
               "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
endif()

strip_symbols(OrbitService)
