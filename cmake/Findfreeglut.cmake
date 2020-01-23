project(freeglut)

find_library(FREEGLUT_SHARED libglut.so)
find_library(FREEGLUT_STATIC libglut.a)
find_path(FREEGLUT_INCLUDE_DIR glut.h PATH_SUFFIXES GL)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(freeglut_shared DEFAULT_MSG FREEGLUT_SHARED
                                  FREEGLUT_INCLUDE_DIR)

if(freeglut_shared_FOUND)
  add_library(freeglut_shared SHARED IMPORTED GLOBAL)
  set_target_properties(freeglut_shared PROPERTIES IMPORTED_LOCATION
                                                   ${FREEGLUT_SHARED})
  target_include_directories(freeglut_shared INTERFACE ${FREEGLUT_INCLUDE_DIR})

  add_library(freeglut::freeglut ALIAS freeglut_shared)
endif()

find_package_handle_standard_args(freeglut_static DEFAULT_MSG FREEGLUT_STATIC
                                  FREEGLUT_INCLUDE_DIR)

if(freeglut_static_FOUND)
  add_library(freeglut_static SHARED IMPORTED GLOBAL)
  set_target_properties(freeglut_static PROPERTIES IMPORTED_LOCATION
                                                   ${FREEGLUT_STATIC})
  target_include_directories(freeglut_static INTERFACE ${FREEGLUT_INCLUDE_DIR})
  target_link_libraries(freeglut_static INTERFACE X11::X11 X11::Xi X11::Xxf86vm)

  add_library(freeglut::freeglut_s ALIAS freeglut_static)
endif()
