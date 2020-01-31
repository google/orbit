if(WIN32)
  find_library(capstone_LIB NAMES capstone_dll)
else()
  find_library(capstone_LIB NAMES libcapstone.a)
endif()
find_path(capstone_INC capstone/capstone.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(capstone DEFAULT_MSG capstone_LIB
                                  capstone_INC)

if(capstone_FOUND)
  add_library(capstone STATIC IMPORTED GLOBAL)
  set_target_properties(capstone PROPERTIES IMPORTED_LOCATION ${capstone_LIB})
  target_include_directories(capstone SYSTEM INTERFACE ${capstone_INC})

  add_library(capstone::capstone ALIAS capstone)
endif()
