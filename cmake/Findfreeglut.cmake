project(freeglut)

#find_library(FREEGLUT_SHARED freeglut${CMAKE_SHARED_LIBRARY_SUFFIX})
find_library(FREEGLUT_LIB freeglut) #${CMAKE_STATIC_LIBRARY_SUFFIX})
find_path(FREEGLUT_INCLUDE_DIR glut.h PATH_SUFFIXES GL)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(freeglut DEFAULT_MSG FREEGLUT_LIB
                                  FREEGLUT_INCLUDE_DIR)


if(freeglut_FOUND)
  add_library(freeglut SHARED IMPORTED GLOBAL)

  if(WIN32)
    set_target_properties(freeglut PROPERTIES IMPORTED_IMPLIB
                                                   "${FREEGLUT_LIB}")
  else()
    set_target_properties(freeglut PROPERTIES IMPORTED_LOCATION
                                                   "${FREEGLUT_LIB}")
  endif()

  target_include_directories(freeglut SYSTEM INTERFACE ${FREEGLUT_INCLUDE_DIR})

  add_library(freeglut::freeglut ALIAS freeglut)
endif()

#find_package_handle_standard_args(freeglut_static DEFAULT_MSG FREEGLUT_STATIC
#                                  FREEGLUT_INCLUDE_DIR)

#if(freeglut_static_FOUND)
#  add_library(freeglut_static SHARED IMPORTED GLOBAL)
#  set_target_properties(freeglut_static PROPERTIES IMPORTED_LOCATION
#                                                   ${FREEGLUT_STATIC})
#  target_include_directories(freeglut_static INTERFACE ${FREEGLUT_INCLUDE_DIR})
#  target_link_libraries(freeglut_static INTERFACE X11::X11 X11::Xi X11::Xxf86vm)
#
#  add_library(freeglut::freeglut_s ALIAS freeglut_static)
#endif()
