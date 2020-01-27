project(breakpad C CXX)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
if (WIN32 AND (build_type_lower STREQUAL "debug" OR (NOT CMAKE_BUILD_TYPE)))
  set(breakpad_POSTFIX "d")
endif()

find_library(breakpad_LIB NAMES "libbreakpad${breakpad_POSTFIX}")
find_path(breakpad_INC common/breakpad_types.h PATH_SUFFIXES google_breakpad)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(breakpad DEFAULT_MSG breakpad_LIB
                                  breakpad_INC)

if(breakpad_FOUND)
  add_library(breakpad STATIC IMPORTED GLOBAL)
  set_target_properties(breakpad PROPERTIES IMPORTED_LOCATION
                                                   ${breakpad_LIB})
  target_include_directories(breakpad SYSTEM INTERFACE ${breakpad_INC})

  add_library(breakpad::breakpad ALIAS breakpad)

  add_library(breakpad_headers INTERFACE IMPORTED GLOBAL)
  target_include_directories(breakpad_headers SYSTEM INTERFACE ${breakpad_INC})
  add_library(breakpad::headers ALIAS breakpad_headers)
endif()

find_library(breakpad_client_LIB NAMES libbreakpad_client${breakpad_POSTFIX})

if(WIN32)
    find_path(breakpad_client_INC client/windows/handler/exception_handler.h)
elseif(UNIX AND NOT APPLE)
    find_path(breakpad_client_INC client/linux/handler/exception_handler.h)
else()
    find_path(breakpad_client_INC client/mac/handler/exception_handler.h)
endif()

find_package_handle_standard_args(breakpad_client DEFAULT_MSG breakpad_client_LIB
                                  breakpad_client_INC)

if(breakpad_client_FOUND)
  add_library(breakpad_client STATIC IMPORTED GLOBAL)
  set_target_properties(breakpad_client PROPERTIES IMPORTED_LOCATION
                                                   ${breakpad_client_LIB})
  target_include_directories(breakpad_client SYSTEM INTERFACE ${breakpad_client_INC})
  target_link_libraries(breakpad_client INTERFACE breakpad::breakpad)

  add_library(breakpad::breakpad_client ALIAS breakpad_client)
endif()