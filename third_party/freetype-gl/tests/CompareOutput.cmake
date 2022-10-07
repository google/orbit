# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

if (NOT IM_COMPARE_EXECUTABLE)
    message(FATAL_ERROR "No \"IM_COMPARE_EXECUTABLE\", pointing to ImageMagic `compare`, given.")
endif()

if (NOT TEST_EXECUTABLE)
    message(FATAL_ERROR "No \"TEST_EXECUTABLE\", pointing to the exe to test, given.")
endif()

if (NOT TEST_OUTPUT_EXPECT)
    message(FATAL_ERROR "No \"TEST_OUTPUT_EXPECT\", pointing to the reference image, given.")
endif()

if (NOT TEST_OUTPUT_CURR)
    message(FATAL_ERROR "No \"TEST_OUTPUT_CURR\", pointing to the screenshot that should be produced, given.")
endif()

if (NOT TEST_OUTPUT_DIFF)
    message(FATAL_ERROR "No \"TEST_OUTPUT_DIFF\", pointing to a diff image that should be produced, given.")
endif()

if (NOT TEST_DISTANCE)
    message(FATAL_ERROR "No \"TEST_DISTANCE\", defining the maximum amount of difference to consider two images equal, given.")
endif()

execute_process(
    COMMAND
        "${TEST_EXECUTABLE}" --screenshot "${TEST_OUTPUT_CURR}"
    RESULT_VARIABLE
        _TEST_RESULT
    OUTPUT_VARIABLE
        _TEST_STDERR
    ERROR_VARIABLE
        _TEST_STDERR
)

if(NOT _TEST_RESULT EQUAL 0)
    file(REMOVE "${TEST_OUTPUT_CURR}")
    message(FATAL_ERROR "Test executable could not be executed. ${TEST_EXECUTABLE}:\n${_TEST_STDERR}")
endif()

execute_process(
    COMMAND
        "${IM_COMPARE_EXECUTABLE}" -metric RMSE "${TEST_OUTPUT_CURR}" "${TEST_OUTPUT_EXPECT}" "${TEST_OUTPUT_DIFF}"
    RESULT_VARIABLE
        _COMPARE_RESULT
    ERROR_VARIABLE
        _COMPARE_STDERR
    OUTPUT_QUIET
)

if(_COMPARE_RESULT EQUAL 0)
    file(REMOVE "${TEST_OUTPUT_CURR}" "${TEST_OUTPUT_DIFF}")
    return()
endif()
if(_COMPARE_RESULT EQUAL 1)
    string(REGEX MATCH "^[0-9]+(\\.[0-9]+)? \\(([0-1]+\\.[0-9]+)\\)$" _DUMMY ${_COMPARE_STDERR})
    set(_DISTANCE ${CMAKE_MATCH_2})

    if(${_DISTANCE} LESS ${TEST_DISTANCE})
        file(REMOVE "${TEST_OUTPUT_CURR}" "${TEST_OUTPUT_DIFF}")
        return()
    else()
        message(FATAL_ERROR "Reference and screenshot too different (RMSE: ${_DISTANCE}).")
    endif()
else()
    message(FATAL_ERROR "Reference and screenshot comparision failed.  ${_COMPARE_STDERR}")
endif()
