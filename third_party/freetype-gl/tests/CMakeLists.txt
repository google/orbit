# Freetype GL - A C OpenGL Freetype engine
#
# Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
# file `LICENSE` for more details.

find_package( ImageMagick COMPONENTS compare REQUIRED )

function(cmp_test TARGET DISTANCE)
    set(_TEST_NAME ${TARGET}-cmp-test)
    add_test(
        NAME
            ${_TEST_NAME}
        COMMAND
            ${CMAKE_COMMAND}
            -DIM_COMPARE_EXECUTABLE=${ImageMagick_compare_EXECUTABLE}
            -DTEST_EXECUTABLE=$<TARGET_FILE:${TARGET}>
            -DTEST_OUTPUT_EXPECT=${freetype-gl_SOURCE_DIR}/doc/images/${TARGET}.png
            -DTEST_OUTPUT_CURR=${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-current.tga
            -DTEST_OUTPUT_DIFF=${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-diff.png
            -DTEST_DISTANCE=${DISTANCE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/CompareOutput.cmake
        WORKING_DIRECTORY
            $<TARGET_FILE_DIR:${TARGET}>
    )
    set_property(
        TEST ${_TEST_NAME}
        PROPERTY ENVIRONMENT
    )
    unset(_TEST_NAME)
endfunction()

cmp_test(ansi 0.01)
if (ANT_TWEAK_BAR_FOUND)
  cmp_test(atb-agg 0.01)
endif(ANT_TWEAK_BAR_FOUND)
cmp_test(benchmark 0.01)
cmp_test(cartoon 0.01)
cmp_test(console 0.01)
cmp_test(cube 0.01)
cmp_test(distance-field 0.01)
cmp_test(distance-field-2 0.01)
cmp_test(distance-field-3 0.01)
cmp_test(embedded-font 0.01)
cmp_test(font 0.01)
cmp_test(gamma 0.01)
cmp_test(glyph 0.01)
if(TARGET freetype-gl-hb)
    cmp_test(harfbuzz 0.01)
    cmp_test(harfbuzz-texture 0.01)
endif()
cmp_test(lcd 0.01)
if(TARGET markup)
    cmp_test(markup 0.01)
endif()
cmp_test(outline 0.01)
cmp_test(subpixel 0.01)
cmp_test(texture 0.01)
