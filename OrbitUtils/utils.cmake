cmake_minimum_required(VERSION 3.6)

# Print all variables containing "var"
function(PrintVars var)
message("==== ${var} ====")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    if(_variableName MATCHES ${var})
        message(STATUS "${_variableName}=${${_variableName}}")
    endif()
endforeach()
endfunction()

# Print all cmake variables
function(PrintAllVars)
message("==== ALL VARS ====")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message("==== /ALL VARS ====")
endfunction(PrintAllVars)