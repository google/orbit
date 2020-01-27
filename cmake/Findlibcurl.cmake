project(libcurl C)

find_library(LIBCURL_STATIC NAMES libcurl${CMAKE_STATIC_LIBRARY_SUFFIX})
find_path(LIBCURL_INCLUDE_DIR curl.h PATH_SUFFIXES curl)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libcurl DEFAULT_MSG LIBCURL_STATIC
                                  LIBCURL_INCLUDE_DIR)

if(libcurl_FOUND)
  add_library(libcurl STATIC IMPORTED GLOBAL)
  set_target_properties(libcurl PROPERTIES IMPORTED_LOCATION
                                               ${LIBCURL_STATIC})
  target_include_directories(libcurl SYSTEM INTERFACE ${LIBCURL_INCLUDE_DIR})

  add_library(libcurl::libcurl ALIAS libcurl)
endif()
