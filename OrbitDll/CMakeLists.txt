set(HEADERS
    OrbitDll.h
)

set(SOURCES
    OrbitDll.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitDll SHARED ${SOURCES} ${HEADERS})

set(MINHOOK_ROOT ${EXTERN_ROOT}/minhook)
target_include_directories(OrbitDll
PRIVATE
    ${MINHOOK_ROOT}/include
)

#message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
#message(STATUS "CMAKE_ARCH=${CMAKE_ARCH}")

#message(STATUS "CMAKE_VS_PLATFORM_NAME=${CMAKE_VS_PLATFORM_NAME}")
#message(STATUS "CMAKE_VS_PLATFORM_TOOLSET=${CMAKE_VS_PLATFORM_TOOLSET}")
#message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

#link_directories(${MINHOOK_ROOT}/lib/debug)

target_link_libraries(OrbitDll
    OrbitCore
	OrbitAsm
	debug "${MINHOOK_ROOT}/lib/Debug/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
	optimized "${MINHOOK_ROOT}/lib/Release/libMinHook.${CMAKE_VS_PLATFORM_NAME}.lib"
	${BREAKPAD_LIBRARIES}
)
