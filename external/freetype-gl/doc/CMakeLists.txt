# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

find_package(Doxygen REQUIRED)

set(FREETYPE_GL_APIDOC_SRC
    Doxyfile.in
    documentation.h
    doxygen.css
    footer.html
    header.html
    layout.xml
    screenshots.h
    style.css
    text-rendering.h
)

file(GLOB FREETYPE_GL_APIDOC_IMG images/*)
file(GLOB FREETYPE_GL_SRC ../*.h ../*.c)
file(GLOB FREETYPE_GL_DEMOS ../demos/*.c)

configure_file(Doxyfile.in Doxyfile)

add_custom_command(
    OUTPUT
        html/index.html
    COMMAND
        ${DOXYGEN_EXECUTABLE} Doxyfile
    DEPENDS
        ${FREETYPE_GL_APIDOC_SRC}
        ${FREETYPE_GL_APIDOC_IMG}
        ${FREETYPE_GL_SRC}
        ${FREETYPE_GL_DEMOS}
)

add_custom_target(doc
    ALL
    DEPENDS html/index.html
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

set_directory_properties(
    PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/html
)
