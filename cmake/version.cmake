# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(VERSION_CMAKE_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function(GenerateVersionFile OUTPUT_FILE INPUT_FILE COMPILE_TARGET)
  get_filename_component(OUTPUT_FILE "${OUTPUT_FILE}" ABSOLUTE)
  get_filename_component(INPUT_FILE "${INPUT_FILE}" ABSOLUTE)

  set(GIT_COMMIT_STATE_FILE "${OUTPUT_FILE}.state")

  set(TARGET_NAME "${OUTPUT_FILE}")

  if("${OUTPUT_FILE}" MATCHES "${CMAKE_BINARY_DIR}*")
    string(LENGTH "${CMAKE_BINARY_DIR}" PREFIX_LENGTH)
    string(SUBSTRING "${OUTPUT_FILE}" ${PREFIX_LENGTH} -1 TARGET_NAME)
  endif()

  string(MAKE_C_IDENTIFIER "${TARGET_NAME}" TARGET_NAME)
  add_custom_target(
    "${TARGET_NAME}" ALL
    DEPENDS ${INPUT_FILE}
    BYPRODUCTS ${OUTPUT_FILE} ${GIT_COMMIT_STATE_FILE}
    COMMENT "Checking Orbit's version by calling git describe"
    COMMAND
      ${CMAKE_COMMAND} -DIN_VERSION_CHECK=TRUE
      "-DGIT_COMMIT_STATE_FILE=${GIT_COMMIT_STATE_FILE}"
      "-DCOMPILER_STRING=${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
      "-DINPUT_FILE=${INPUT_FILE}" "-DOUTPUT_FILE=${OUTPUT_FILE}" -P
      "${VERSION_CMAKE_SCRIPT}")

  if(COMPILE_TARGET)
    add_dependencies(${COMPILE_TARGET} "${TARGET_NAME}")
  endif()
endfunction()

function(DoGenerateVersionFile OUTPUT_FILE INPUT_FILE GIT_COMMIT_STATE_FILE
         COMMIT_HASH)
  file(WRITE "${GIT_COMMIT_STATE_FILE}" "${COMMIT_HASH}")
  execute_process(
    COMMAND "${GIT_COMMAND}" "describe" "--always" "--match" "1.*"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_VARIABLE VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCH "([0-9]+)\\." MAJOR_VERSION "${VERSION_STRING}")
  set(MAJOR_VERSION "${CMAKE_MATCH_1}")

  if(NOT MAJOR_VERSION)
    set(MAJOR_VERSION 0)
  endif()

  string(REGEX MATCH "\\.([0-9]+)" MINOR_VERSION "${VERSION_STRING}")
  set(MINOR_VERSION "${CMAKE_MATCH_1}")

  if(NOT MINOR_VERSION)
    set(MINOR_VERSION 0)
  endif()

  set(PATCH_VERSION "0")

  string(REGEX MATCH "-([0-9]+)-" MINOR_PATCH_VERSION "${VERSION_STRING}")
  if(MINOR_PATCH_VERSION)
    set(MINOR_PATCH_VERSION "${CMAKE_MATCH_1}")
  else()
    set(MINOR_PATCH_VERSION "0")
  endif()

  cmake_host_system_information(RESULT BUILD_MACHINE_STRING QUERY HOSTNAME)
  cmake_host_system_information(RESULT BUILD_OS_NAME QUERY OS_NAME)
  cmake_host_system_information(RESULT BUILD_OS_RELEASE QUERY OS_RELEASE)
  cmake_host_system_information(RESULT BUILD_OS_VERSION QUERY OS_VERSION)
  cmake_host_system_information(RESULT BUILD_OS_PLATFORM QUERY OS_PLATFORM)
  string(TIMESTAMP BUILD_TIMESTAMP_STRING UTC)

  configure_file("${INPUT_FILE}" "${OUTPUT_FILE}")
endfunction()

if(IN_VERSION_CHECK)
  find_program(GIT_COMMAND git REQUIRED)

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
