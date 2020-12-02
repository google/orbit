# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if(NOT WIN32)
  include(ProcessorCount)
  ProcessorCount(CPU_COUNT)

  find_package(Python3 COMPONENTS Interpreter)
  find_program(iwyu_tool_py NAMES iwyu_tool)

  if (iwyu_tool_py AND TARGET Python3::Interpreter)
    # Create a custom version of compile_commands.json which does not include any compilation units from third_party
    # or any generated cpp-files. This will be used by include-what-you-use.
    add_custom_command(OUTPUT iwyu_commands.json
      COMMAND jq 'map(select(.file | test(\"third_party|/build_|/build/\") != true))' > iwyu_commands.json < compile_commands.json
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # That's the public facing include-what-you-use target which can be invoked by `make iwyu`.
    # It generates the include-what-you-use report and writes it into iwyu_results.txt in the
    # build folder.
    add_custom_target(iwyu
      COMMAND "${Python3_EXECUTABLE}" "${iwyu_tool_py}" -p "${CMAKE_BINARY_DIR}/iwyu_commands.json" -o clang 
        -j ${CPU_COUNT} -- -w -Xiwyu "--mapping_file=${CMAKE_SOURCE_DIR}/contrib/iwyu/qt5_14.imp"
        -Xiwyu "--mapping_file=${CMAKE_SOURCE_DIR}/contrib/iwyu/orbit.imp"
        -Xiwyu --cxx17ns -Xiwyu --max_line_length=120 -Xiwyu --no_fwd_decls | sed s|${CMAKE_SOURCE_DIR}/||
        > ${CMAKE_BINARY_DIR}/iwyu_results.txt
      DEPENDS iwyu_commands.json
      COMMENT "Running include-what-you-use"
      VERBATIM)
  endif()
endif()

# include-what-you-use needs full source-code visibilty, which means
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


