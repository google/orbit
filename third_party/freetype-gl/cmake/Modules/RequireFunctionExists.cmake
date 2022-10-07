# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

#.rst:
# RequireFunctionExists
# ---------------------
#
# Require that a C function can be linked
#
# REQUIRE_FUNCTION_EXISTS(<function> <variable>)
#
# Uses check_function_exists internally and aborts the configure process with a
# Fatal error when the function can not be linked.
#
# See the :module:`CheckFunctionExists` module for more details.

include(CheckFunctionExists)

macro(REQUIRE_FUNCTION_EXISTS FUNCTION_NAME VARIABLE)
    check_function_exists(${FUNCTION_NAME} ${VARIABLE})

    if(NOT ${VARIABLE})
        message(FATAL_ERROR "Required `${FUNCTION_NAME}` could not be linked")
    endif()
endmacro()
