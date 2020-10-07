# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(webUI CXX)

add_executable(QWebChannelExtractor QWebChannelExtractor.cpp)
target_link_libraries(QWebChannelExtractor PUBLIC Qt5::WebChannel)
set(q_webchannel_js_output "${CMAKE_CURRENT_BINARY_DIR}/qtwebchannel/qwebchannel.js")

add_custom_command(OUTPUT "${q_webchannel_js_output}"
                   COMMAND "$<TARGET_FILE:QWebChannelExtractor>"
                   ARGS "${q_webchannel_js_output}"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS QWebChannelExtractor
                   COMMENT "Extracting qwebchannel.js from Qt5::WebChannel library")

if(WIN32)
  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET QWebChannelExtractor
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:QWebChannelExtractor>"
    COMMENT "Running windeployqt for QWebChannelExtractor...")
endif()

if(WIN32)
  find_program(NPM npm.cmd)
else()
  find_program(NPM npm)
endif()

set(NODE_AWARE_PATH "$ENV{PATH}" CACHE STRING "The state of PATH during config phase")

execute_process(COMMAND ${CMAKE_COMMAND} -E env "PATH=${NODE_AWARE_PATH}"
                        ${NPM} install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/WebUI.qrc
                   COMMAND ${CMAKE_COMMAND}
                     -E env "ORBIT_BUILD_FOLDER=${CMAKE_BINARY_DIR}"
                            "PATH=${NODE_AWARE_PATH}"
                     ${NPM} run build
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   MAIN_DEPENDENCY webpack.config.js
                   DEPENDS ${q_webchannel_js_output}
                           webpack.config.js
                           package.json
                           package-lock.json
                           src/Heatmap.ts
                           src/Heatmap.css
                           src/CodeView.ts
                           src/CodeView.hbs
                           src/CodeView.css
                           src/x86asm.ts
                   )

add_library(WebUI OBJECT)
target_sources(WebUI PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/dist/WebUI.qrc)
target_link_libraries(WebUI PUBLIC Qt5::Core)
set_target_properties(WebUI PROPERTIES AUTORCC 1)
