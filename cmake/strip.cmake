# * Strips debug symbols from an executable target
# * Copies debug symbols into an extra debug-symbol-file
# * Adds reference to this debug symbol from to the executable
# * Does not support Windows / Is not needed on Windows
function(strip_symbols target)
  if(NOT WIN32)
    add_custom_command(
            TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} --only-keep-debug "$<TARGET_FILE_NAME:${target}>"
              "$<TARGET_FILE_NAME:${target}>.debug"
      COMMAND ${CMAKE_OBJCOPY} --strip-debug "$<TARGET_FILE_NAME:${target}>"
      COMMAND
        ${CMAKE_OBJCOPY}
        --add-gnu-debuglink="$<TARGET_FILE_NAME:${target}>.debug"
        "$<TARGET_FILE_NAME:${target}>"
      WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>")
  endif()
endfunction()
