# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

function(DoGenerateThirdPartyLicenseFile OUTPUT_FILE INPUT_DIRECTORY)
  get_filename_component(INPUT_DIRECTORY "${INPUT_DIRECTORY}" ABSOLUTE)
  get_filename_component(OUTPUT_FILE "${OUTPUT_FILE}" ABSOLUTE)

  file(GLOB_RECURSE files RELATIVE "${INPUT_DIRECTORY}" "${INPUT_DIRECTORY}/*")
  file(REMOVE "${OUTPUT_FILE}")

  foreach(file ${files})
    file(APPEND "${OUTPUT_FILE}" "================================================================================\n")
    file(APPEND "${OUTPUT_FILE}" "${file}:\n\n")
    file(READ "${INPUT_DIRECTORY}/${file}" content)
    file(APPEND "${OUTPUT_FILE}" "${content}")
    file(APPEND "${OUTPUT_FILE}" "\n\n\n")
  endforeach()
endfunction()

set(GEN_THIRD_PARTY_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function(GenerateThirdPartyLicenseFile OUTPUT_FILE INPUT_DIRECTORY)
  get_filename_component(INPUT_DIRECTORY "${INPUT_DIRECTORY}" ABSOLUTE)
  get_filename_component(OUTPUT_FILE "${OUTPUT_FILE}" ABSOLUTE)
  add_custom_target(ThirdPartyLicenseFile ALL COMMAND ${CMAKE_COMMAND} -DIN_GENERATE_THIRD_PARTY_LICENSE_FILE=TRUE -DINPUT_DIRECTORY="${INPUT_DIRECTORY}" -DOUTPUT_FILE="${OUTPUT_FILE}" -P "${GEN_THIRD_PARTY_SCRIPT}" BYPRODUCTS "${OUTPUT_FILE}" COMMENT "Generating third-party LICENSE file...")
endfunction()

if(IN_GENERATE_THIRD_PARTY_LICENSE_FILE)
  DoGenerateThirdPartyLicenseFile("${OUTPUT_FILE}" "${INPUT_DIRECTORY}")
endif()

