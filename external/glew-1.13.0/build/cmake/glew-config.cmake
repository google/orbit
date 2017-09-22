# This config-module creates the following import libraries:
#
# - GLEW::glew and GLEW::glewmx shared libs
# - GLEW::glew_s and GLEW::glewmx_s static libs
#
# Additionally GLEW::GLEW and GLEW::GLEWMX will be created as an
# copy of either the shared (default) or the static libs.
#
# Dependending on the setting of BUILD_SHARED_LIBS at GLEW build time
# either the static or shared versions may not be available.
#
# Set GLEW_USE_STATIC_LIBS to OFF or ON to force using the shared
# or static libs for GLEW::GLEW and GLEW::GLEWMX
#

include(${CMAKE_CURRENT_LIST_DIR}/glew-targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CopyImportedTargetProperties.cmake)

# decide which import library (glew/glew_s and glewmx/glewmx_s)
# needs to be copied to GLEW::GLEW and GLEW::GLEWMX
set(_glew_target_postfix "")
set(_glew_target_type SHARED)
if(DEFINED GLEW_USE_STATIC_LIBS)
  # if defined, use only static or shared
  if(GLEW_USE_STATIC_LIBS)
    set(_glew_target_postfix "_s")
  endif()
  # else use static only if no shared
elseif(NOT TARGET GLEW::glew AND TARGET GLEW::glew_s)
  set(_glew_target_postfix "_s")
endif()
if(_glew_target_postfix STREQUAL "")
  set(_glew_target_type SHARED)
else()
  set(_glew_target_type STATIC)
endif()

# CMake doesn't allow creating ALIAS lib for an IMPORTED lib
# so create imported ones and copy the properties
foreach(_glew_target glew glewmx)
  set(_glew_src_target "GLEW::${_glew_target}${_glew_target_postfix}")
  string(TOUPPER "GLEW::${_glew_target}" _glew_dest_target)
  add_library(${_glew_dest_target} ${_glew_target_type} IMPORTED)
  # message(STATUS "add_library(${_glew_dest_target} ${_glew_target_type} IMPORTED)")
  copy_imported_target_properties(${_glew_src_target} ${_glew_dest_target})
endforeach()
