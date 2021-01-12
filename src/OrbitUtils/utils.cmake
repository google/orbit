cmake_minimum_required(VERSION 3.6)

# To enable PrintVars output, set PRINT_VAR_OUTPUT to ON
set(PRINT_VAR_OUTPUT OFF)

# Print all variables containing "var"
function(PrintVars var)
if(PRINT_VAR_OUTPUT)
message("/=== ${var} ===")
string( TOLOWER ${var} var_lower )
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    string( TOLOWER ${_variableName} cmake_var_lower)
    if(${cmake_var_lower} MATCHES ${var_lower})
        message(STATUS "${_variableName}=${${_variableName}}")
    endif()
endforeach()
message("\\=== ${var} ===\n")
endif(PRINT_VAR_OUTPUT)
endfunction()

# Print all cmake variables
function(PrintAllVars)
message("==== ALL VARS ====")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message("^=== /ALL VARS ====")
endfunction(PrintAllVars)