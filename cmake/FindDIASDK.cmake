project(DIASDK)

set(MSVC_DIA_SDK_DIR "$ENV{VSINSTALLDIR}DIA SDK" CACHE PATH "Path to the DIA SDK")

if (IS_DIRECTORY ${MSVC_DIA_SDK_DIR})
    set(DIA_PLATFORM_NAME "")

    if(CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
      set(DIA_PLATFORM_NAME "amd64")
    endif()

    find_library(DIASDK_LIB NAMES diaguids.lib HINTS "${MSVC_DIA_SDK_DIR}/lib/${DIA_PLATFORM_NAME}")
    find_path(DIASDK_INC dia2.h HINTS "${MSVC_DIA_SDK_DIR}/include")

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DIASDK DEFAULT_MSG DIASDK_LIB
                                  DIASDK_INC)

if(DIASDK_FOUND)
  add_library(DIASDK STATIC IMPORTED GLOBAL)
  set_target_properties(DIASDK PROPERTIES IMPORTED_LOCATION
                                                   ${DIASDK_LIB})
  target_include_directories(DIASDK SYSTEM INTERFACE ${DIASDK_INC})

  add_library(DIASDK::DIASDK ALIAS DIASDK)
endif()