project(OrbitDll CXX)

add_library(OrbitDll SHARED OrbitDll.cpp OrbitDll.h)

target_link_libraries(
  OrbitDll
  PUBLIC OrbitCore OrbitAsm minhook::minhook breakpad::breakpad)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ORBIT_DLL_NAME "Orbit64")
else()
  set(ORBIT_DLL_NAME "Orbit32")
endif()

set_target_properties(OrbitDll PROPERTIES OUTPUT_NAME ${ORBIT_DLL_NAME})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # Disable incremental linking as it hides the real address of functions. This
  # is annoying mainly for the OrbitProlog and OrbitEpilog related code.
  set_target_properties(OrbitDll PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL:NO")
  set_target_properties(OrbitDll PROPERTIES LINK_FLAGS_RELWITHDEBINFO
                                            "/INCREMENTAL:NO")
endif()

target_compile_definitions(OrbitDll PRIVATE WIN32_LEAN_AND_MEAN)
target_compile_definitions(OrbitDll PRIVATE NOMINMAX)
