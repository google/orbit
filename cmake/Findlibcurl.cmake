project(libcurl C)

include(FindPkgConfig)
pkg_search_module(LIBCURL libcurl)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libcurl DEFAULT_MSG LIBCURL_LIBRARIES
                                  LIBCURL_INCLUDE_DIRS)

if(libcurl_FOUND)
  add_library(libcurl STATIC IMPORTED GLOBAL)
  set_target_properties(libcurl PROPERTIES IMPORTED_LOCATION
                                               ${LIBCURL_LINK_LIBRARIES})
  target_include_directories(libcurl INTERFACE ${LIBCURL_INCLUDE_DIR})

  add_library(libcurl::libcurl ALIAS libcurl)
endif()
