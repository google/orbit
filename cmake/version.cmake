# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(VERSION_CMAKE_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function(GenerateVersionFile OUTPUT_FILE INPUT_FILE)
  get_filename_component(OUTPUT_FILE "${OUTPUT_FILE}" ABSOLUTE)
  get_filename_component(INPUT_FILE "${INPUT_FILE}" ABSOLUTE)

  set(GIT_COMMIT_STATE_FILE "${OUTPUT_FILE}.state")

  string(MAKE_C_IDENTIFIER "${OUTPUT_FILE}" TARGET_NAME)
  add_custom_target(
    "check_git_version_${TARGET_NAME}" ALL
    DEPENDS ${INPUT_FILE}
    BYPRODUCTS ${OUTPUT_FILE} ${GIT_COMMIT_STATE_FILE}
    COMMENT "Checking Orbit's version by calling git describe"
    COMMAND
      ${CMAKE_COMMAND} -DIN_VERSION_CHECK=TRUE
      "-DGIT_COMMIT_STATE_FILE=${GIT_COMMIT_STATE_FILE}"
      "-DINPUT_FILE=${INPUT_FILE}" "-DOUTPUT_FILE=${OUTPUT_FILE}" -P
      "${VERSION_CMAKE_SCRIPT}")
endfunction()

function(DoGenerateVersionFile OUTPUT_FILE INPUT_FILE GIT_COMMIT_STATE_FILE
         COMMIT_HASH)
  file(WRITE "${GIT_COMMIT_STATE_FILE}" "${COMMIT_HASH}")
  execute_process(
    COMMAND "${GIT_COMMAND}" "describe" "--tags" "--always"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_VARIABLE VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCH "([0-9]+)\\." MAJOR_VERSION "${VERSION_STRING}")
  set(MAJOR_VERSION "${CMAKE_MATCH_1}")

  string(REGEX MATCH "\\.([0-9]+)" MINOR_VERSION "${VERSION_STRING}")
  set(MINOR_VERSION "${CMAKE_MATCH_1}")

  set(PATCH_VERSION "0")

  string(REGEX MATCH "-([0-9]+)-" MINOR_PATCH_VERSION "${VERSION_STRING}")
  if(MINOR_PATCH_VERSION)
    set(MINOR_PATCH_VERSION "${CMAKE_MATCH_1}")
  else()
    set(MINOR_PATCH_VERSION "0")
  endif()

  configure_file("${INPUT_FILE}" "${OUTPUT_FILE}")
endfunction()

if(IN_VERSION_CHECK)
  find_program(GIT_COMMAND git)

  execute_process(
    COMMAND "${GIT_COMMAND}" "show" "-s" "--format=%H"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_VARIABLE COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT EXISTS "${OUTPUT_FILE}"
     OR NOT EXISTS "${GIT_COMMIT_STATE_FILE}"
     OR "${INPUT_FILE}" IS_NEWER_THAN "${OUTPUT_FILE}")
    dogenerateversionfile("${OUTPUT_FILE}" "${INPUT_FILE}"
                          "${GIT_COMMIT_STATE_FILE}" "${COMMIT_HASH}")
  else()
    file(READ "${GIT_COMMIT_STATE_FILE}" STATE)
    if(NOT "${STATE}" STREQUAL "${COMMIT_HASH}")
      dogenerateversionfile("${OUTPUT_FILE}" "${INPUT_FILE}"
                            "${GIT_COMMIT_STATE_FILE}" "${COMMIT_HASH}")
    endif()
  endif()
endif()
