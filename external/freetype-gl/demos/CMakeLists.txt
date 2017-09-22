# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

macro(CREATE_DEMO _target _sources)
    add_executable(${_target} ${_sources} $<TARGET_OBJECTS:demo-utils>)
    if(${_target} MATCHES "^harfbuzz")
        target_link_libraries(${_target}
            freetype-gl-hb
            ${HARFBUZZ_LIBRARIES}
        )
    else()
        target_link_libraries(${_target}
            freetype-gl
        )
    endif()
    target_link_libraries(${_target}
        ${GLFW3_LIBRARY}
        ${OPENGL_LIBRARY}
        ${FREETYPE_LIBRARIES}
        ${MATH_LIBRARY}
        ${GLEW_LIBRARY}
    )

    if(MSVC AND NOT (MSVC_VERSION LESS 1900))
        # prevent error LNK2019: unresolved external symbol _sprintf referenced in function __bdf_parse_properties
        # see http://stackoverflow.com/a/32418900/469659
        target_link_libraries(${_target} "legacy_stdio_definitions.lib")
    endif()
endmacro()

if(WIN32 OR WIN64)
    set(ENV{ANT_TWEAK_BAR_ROOT}
        ${CMAKE_CURRENT_SOURCE_DIR}/../windows/AntTweakBar
    )
endif()

# Find dependencies
find_package(glfw3 3.2 REQUIRED)
find_package(AntTweakBar)
find_package(FontConfig)

include_directories(
    ${GLFW3_INCLUDE_DIR}
)

link_directories(
    ${GLFW3_LIBRARY_DIR}
)

add_library(demo-utils OBJECT
    mat4.h
    mat4.c
    screenshot-util.h
    screenshot-util.c
    shader.h
    shader.c
)

add_custom_command(
    DEPENDS
        makefont
        ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    COMMAND
        ../$<$<BOOL:MSVC>:$(Configuration)/>makefont
    ARGS
        --font fonts/Vera.ttf
        --header ${CMAKE_CURRENT_SOURCE_DIR}/vera-16.h
        --size 16
        --variable font
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/vera-16.h
)

add_custom_target(generate-embedded-font
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vera-16.h
)

create_demo(embedded-font embedded-font.c)
add_dependencies(embedded-font generate-embedded-font)

create_demo(texture texture.c)
create_demo(font font.c)
create_demo(benchmark benchmark.c)
create_demo(console console.c)
create_demo(cube cube.c)
create_demo(glyph glyph.c)
create_demo(cartoon cartoon.c)
create_demo(outline outline.c)
create_demo(lcd lcd.c)
create_demo(gamma gamma.c)
create_demo(ansi ansi.c)
create_demo(subpixel subpixel.c)
create_demo(distance-field distance-field.c)
create_demo(distance-field-2 distance-field-2.c)
create_demo(distance-field-3 distance-field-3.c)

if(FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIR})
    create_demo(markup markup.c)
    target_link_libraries(markup ${FONTCONFIG_LIBRARY})
endif()

if(ANT_TWEAK_BAR_FOUND)
    include_directories(${ANT_TWEAK_BAR_INCLUDE_PATH})
    create_demo(atb-agg atb-agg.c)
    target_link_libraries(atb-agg ${ANT_TWEAK_BAR_LIBRARY})
    set_target_properties(atb-agg PROPERTIES LINKER_LANGUAGE CXX)
endif()

if(TARGET freetype-gl-hb)
    create_demo(harfbuzz harfbuzz.c)
    create_demo(harfbuzz-texture harfbuzz-texture.c)
    target_include_directories(harfbuzz
        BEFORE
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../harfbuzz
        PRIVATE ${HARFBUZZ_INCLUDE_DIRS}
    )
    target_include_directories(harfbuzz-texture
        BEFORE
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../harfbuzz
        PRIVATE ${HARFBUZZ_INCLUDE_DIRS}
    )
endif()

# Copy font and shaders into build directory for in-place testing
file(COPY ../fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        file(COPY ../fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config})
        file(COPY ../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config})
        file(COPY ../shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config})
    endforeach()
endif()
