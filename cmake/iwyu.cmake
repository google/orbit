# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if(NOT WIN32)
  include(ProcessorCount)
  ProcessorCount(cpu_count)

  find_package(Python3 COMPONENTS Interpreter)
  find_program(iwyu_tool_py NAMES iwyu_tool iwyu_tool.py)
  set(fix_includes_py ${CMAKE_SOURCE_DIR}/third_party/include-what-you-use/fix_includes.py)
  find_program(clang_format_diff NAMES clang-format-diff-9 clang-format-diff)
  find_program(combine_diff NAMES combinediff)
  find_program(filter_diff NAMES filterdiff)

  if (iwyu_tool_py AND TARGET Python3::Interpreter)
    # Create a custom version of compile_commands.json which does not include any compilation units from third_party
    # or any generated cpp-files. This will be used by include-what-you-use.
    add_custom_command(OUTPUT iwyu_commands.json
      COMMAND jq 'map(select(.file | test(\"third_party|/build_|/build/\") != true)) ' > iwyu_commands.json < compile_commands.json
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # That's the public facing include-what-you-use target which can be invoked by `make iwyu`.
    # It generates the include-what-you-use report and writes it into `include-what-you-use.log` in the
    # build folder. Furthermore it creates a unified diff `iwyu.diff` in the build folder which can be
    # applied to the code base.
    add_custom_command(OUTPUT include-what-you-use.log
      COMMAND "${Python3_EXECUTABLE}" "${iwyu_tool_py}" -p iwyu_commands.json -o iwyu
        -j ${cpu_count} -- -w
        -Xiwyu "--mapping_file=${CMAKE_SOURCE_DIR}/contrib/iwyu/orbit.imp"
        -Xiwyu --cxx17ns -Xiwyu --max_line_length=120 -Xiwyu --no_fwd_decls > include-what-you-use.log
      DEPENDS iwyu_commands.json
      "${CMAKE_SOURCE_DIR}/contrib/iwyu/orbit.imp"
      "${CMAKE_SOURCE_DIR}/contrib/iwyu/qt5_14.imp"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Running include-what-you-use"
      VERBATIM)

    add_custom_command(OUTPUT iwyu_unformatted.diff
      COMMAND "${Python3_EXECUTABLE}" "${fix_includes_py}" -n -p ${CMAKE_SOURCE_DIR}
        < include-what-you-use.log > iwyu_unformatted.diff
      DEPENDS include-what-you-use.log
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      VERBATIM)

    set(wrapper_content "\
#!/bin/bash

set -euo pipefail

function reverse_apply {
  git apply -R -p1 \"${CMAKE_BINARY_DIR}/iwyu_unformatted.diff\"
}

if git apply -p1 \"${CMAKE_BINARY_DIR}/iwyu_unformatted.diff\"; then
  trap reverse_apply EXIT
  $@
fi
")
    file(GENERATE OUTPUT clang_format_wrapper.sh CONTENT "${wrapper_content}")

    add_custom_command(OUTPUT iwyu_format_additions.diff
      COMMAND /bin/bash ${CMAKE_BINARY_DIR}/clang_format_wrapper.sh ${clang_format_diff} -p1
        < ${CMAKE_BINARY_DIR}/iwyu_unformatted.diff
        | ${filter_diff} --addoldprefix=a/ --addnewprefix=b/ --clean --remove-timestamps
        > ${CMAKE_BINARY_DIR}/iwyu_format_additions.diff
      DEPENDS ${CMAKE_BINARY_DIR}/iwyu_unformatted.diff ${CMAKE_BINARY_DIR}/clang_format_wrapper.sh
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      VERBATIM)

    add_custom_command(OUTPUT iwyu.diff
      COMMAND ${combine_diff} -p1 iwyu_unformatted.diff iwyu_format_additions.diff > iwyu.diff
      DEPENDS iwyu_format_additions.diff
      VERBATIM)

    add_custom_target(iwyu DEPENDS iwyu.diff COMMENT "Running iwyu. Check iwyu.diff for final results.")
  endif()
endif()

# include-what-you-use needs full source-code visibility, which means
# we don't need to compile all the code, but we need to run all code
# generators. The following functions takes care of Qt's UI files.
# It calls the generator without compiling the related classes.
# That's a little bit hacky, but that way we avoid building almost 
# the whole project.
function(iwyu_add_dependency TARGET)
  if (TARGET iwyu)
    if(AUTOGEN_BUILD_DIR)
      set(include_dir "${AUTOGEN_BUILD_DIR}/include")
    else()
      set(include_dir "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_autogen/include")
    endif()

    get_target_property(sources ${TARGET} SOURCES)
    list(FILTER sources INCLUDE REGEX "\.ui$")
    list(LENGTH sources sources_length)

    if(${sources_length} EQUAL 0)
      return()
    endif()

    set(headers ${sources})
    list(TRANSFORM headers REPLACE "\.ui$" ".h")
    list(TRANSFORM headers PREPEND "${include_dir}/ui_")

    math(EXPR sources_max_idx "${sources_length} - 1")
    foreach(idx RANGE ${sources_max_idx})
      list(GET sources ${idx} ui_file)
      list(GET headers ${idx} header_file)

      find_program(UIC NAMES uic)
      add_custom_command(OUTPUT ${header_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${include_dir}
        COMMAND ${UIC} -o ${header_file} ${ui_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM)
    endforeach()

    add_custom_target(${TARGET}_iwyu_ui DEPENDS ${headers})
    add_dependencies(iwyu ${TARGET}_iwyu_ui)
  endif()
endfunction()


