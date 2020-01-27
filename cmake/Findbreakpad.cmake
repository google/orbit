project(breakpad C CXX)

find_library(breakpad_LIB NAMES libbreakpad)
find_path(breakpad_INC common/breakpad_types.h PATH_SUFFIXES google_breakpad)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(breakpad DEFAULT_MSG breakpad_LIB
                                  breakpad_INC)

if(breakpad_FOUND)
  add_library(breakpad STATIC IMPORTED GLOBAL)
  set_target_properties(breakpad PROPERTIES IMPORTED_LOCATION
                                                   ${breakpad_LIB})
  target_include_directories(breakpad INTERFACE ${breakpad_INC})

  add_library(breakpad::breakpad ALIAS breakpad)

  add_library(breakpad_headers INTERFACE IMPORTED GLOBAL)
  target_include_directories(breakpad_headers INTERFACE ${breakpad_INC})
  add_library(breakpad::headers ALIAS breakpad_headers)
endif()

find_library(breakpad_client_LIB NAMES libbreakpad_client)

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
  target_include_directories(breakpad_client INTERFACE ${breakpad_client_INC})
  target_link_libraries(breakpad_client INTERFACE breakpad::headers)

  add_library(breakpad::breakpad_client ALIAS breakpad_client)
endif()