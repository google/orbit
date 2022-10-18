#
# Try to find AntTweakBar library and include path.
# Once done this will define
#
# ANT_TWEAK_BAR_FOUND
# ANT_TWEAK_BAR_INCLUDE_PATH
# ANT_TWEAK_BAR_LIBRARY
#

IF (WIN32)
    FIND_PATH( ANT_TWEAK_BAR_INCLUDE_PATH AntTweakBar.h
        PATHS
        $ENV{ANT_TWEAK_BAR_ROOT}/include
        DOC "The directory where AntTweakBar.h resides")

    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(ANT_TWEAK_BAR_LIBRARY_NAME "AntTweakBar64")
    ELSE()
        SET(ANT_TWEAK_BAR_LIBRARY_NAME "AntTweakBar")
    ENDIF()

    FIND_LIBRARY( ANT_TWEAK_BAR_LIBRARY ${ANT_TWEAK_BAR_LIBRARY_NAME}
        PATHS
        $ENV{ANT_TWEAK_BAR_ROOT}/lib
        DOC "The AntTweakBar library")
ELSE ()
    FIND_PATH( ANT_TWEAK_BAR_INCLUDE_PATH AntTweakBar.h
        PATHS
        /usr/local/include
        /usr/X11/include
        /usr/include)

    FIND_LIBRARY( ANT_TWEAK_BAR_LIBRARY AntTweakBar
        PATHS
        /usr/local
        /usr/X11
        /usr
        PATH_SUFFIXES
        lib64
        lib
        dylib
)
ENDIF ()

SET(ANT_TWEAK_BAR_FOUND "NO")
IF (ANT_TWEAK_BAR_INCLUDE_PATH AND ANT_TWEAK_BAR_LIBRARY)
    SET(ANT_TWEAK_BAR_FOUND "YES")
ENDIF (ANT_TWEAK_BAR_INCLUDE_PATH AND ANT_TWEAK_BAR_LIBRARY)

INCLUDE(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AntTweakBar DEFAULT_MSG ANT_TWEAK_BAR_INCLUDE_PATH ANT_TWEAK_BAR_LIBRARY)
