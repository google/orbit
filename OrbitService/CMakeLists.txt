project(OrbitService)

add_executable(OrbitService)

target_sources(OrbitService PRIVATE main.cpp OrbitService.cpp)
target_sources(OrbitService PUBLIC OrbitService.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set_target_properties(OrbitService PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(OrbitService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries (OrbitService PRIVATE OrbitGl OrbitCore)
# libcurl::libcurl breakpad::breakpad_client
#  imgui::imgui freetype-gl::freetype-gl freeglut::freeglut GLEW::glew) 

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Administrator privileges
    SET_TARGET_PROPERTIES(OrbitService PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
endif()

if(WIN32)
    add_custom_command(TARGET OrbitService POST_BUILD
        # External
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/libcurl.dll  $<TARGET_FILE_DIR:OrbitService> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/imgui.dll    $<TARGET_FILE_DIR:OrbitService> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/zlib1.dll    $<TARGET_FILE_DIR:OrbitService> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/bz2.dll      $<TARGET_FILE_DIR:OrbitService> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/freeglut.dll $<TARGET_FILE_DIR:OrbitService>     	
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/glew32.dll   $<TARGET_FILE_DIR:OrbitService> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/libpng16.dll $<TARGET_FILE_DIR:OrbitService>     
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXTERN_ROOT}/vcpkg/installed/x64-windows/bin/freetype.dll $<TARGET_FILE_DIR:OrbitService> 

        # OrbitDll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/OrbitDll/$<CONFIG>/Orbit64.dll"        $<TARGET_FILE_DIR:OrbitService>
        #COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ORBIT_ROOT}/build/x86/OrbitDll/Release/Orbit32.dll"      $<TARGET_FILE_DIR:OrbitService>
    )
endif()