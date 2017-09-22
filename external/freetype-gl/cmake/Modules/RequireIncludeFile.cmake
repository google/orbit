# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

#.rst:
# RequireIncludeFile
# ------------------
#
# Provides a macro to require the existence of a ``C`` header.
#
# .. command:: REQUIRE_INCLUDE_FILE
#
#  ::
#
#    REQUIRE_INCLUDE_FILE(<include> VARIABLE [<FLAGS>])
#
#  Uses check_include_file internally and aborts the configure process with a
#  Fatal error when the header was not found.
#
# See the :module:`CheckIncludeFile` module for more details.

include(CheckIncludeFile)

macro(REQUIRE_INCLUDE_FILE INCLUDE_FILE VARIABLE)
    if(${ARGC} EQUAL 3)
        check_include_file(${INCLUDE_FILE} ${VARIABLE} ${ARGV2})
    else()
        check_include_file(${INCLUDE_FILE} ${VARIABLE})
    endif()

    if(NOT ${VARIABLE})
        message(FATAL_ERROR "Required `${INCLUDE_FILE}` include file not found")
    endif()
endmacro()
