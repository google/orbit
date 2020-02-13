function(find_dia_sdk_path version_year)
  if(EXISTS "C:/Program Files (x86)/Microsoft Visual Studio/${version_year}/Professional/DIA SDK")
    set(MSVC_DIA_SDK_DIR
        "C:/Program Files (x86)/Microsoft Visual Studio/${version_year}/Professional/DIA SDK"
        CACHE PATH "The DIA SDK path" FORCE)
  elseif(EXISTS "C:/Program Files (x86)/Microsoft Visual Studio/${version_year}/Community/DIA SDK")
    set(MSVC_DIA_SDK_DIR
        "C:/Program Files (x86)/Microsoft Visual Studio/${version_year}/Community/DIA SDK"
        CACHE PATH "The DIA SDK path" FORCE)
  else()
      MESSAGE( FATAL_ERROR "Dia SDK directory could not be found." )
  endif()
  MESSAGE( STATUS "MSVC_DIA_SDK_DIR set to:" ${MSVC_DIA_SDK_DIR} )
endfunction()
