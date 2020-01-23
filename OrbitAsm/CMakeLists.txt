project(OrbitAsm CXX ASM_MASM)

add_library(OrbitAsm STATIC)

target_sources(OrbitAsm PUBLIC OrbitAsm.h)
target_sources(OrbitAsm PRIVATE OrbitAsm.cpp)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_sources(OrbitAsm PRIVATE Orbit.asm)
endif()

target_link_libraries(OrbitAsm PUBLIC minhook::minhook)
target_include_directories(OrbitAsm PUBLIC ${CMAKE_CURRENT_LIST_DIR})
