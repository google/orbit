project(OrbitQt CXX)

# # Disable incremental linking as it hides the real address of functions. #
# This is annoying mainly for the OrbitProlog and OrbitEpilog related code.
# if(WIN32) set( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO") set(
# ORBIT_DEBUG_LINKER_FLAGS "/debug /INCREMENTAL:NO" )

# # debug message("Changing Debug linker flags from
# '${CMAKE_EXE_LINKER_FLAGS_DEBUG}' to '${ORBIT_DEBUG_LINKER_FLAGS}'")
# set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${ORBIT_DEBUG_LINKER_FLAGS} )
# message("CMAKE_EXE_LINKER_FLAGS_DEBUG=${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

# # rewithdebinfo PrintVars(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO)
# message("Changing RelWithDebInfo linker flags from
# '${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}' to '${ORBIT_DEBUG_LINKER_FLAGS}'")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${ORBIT_DEBUG_LINKER_FLAGS} )
# endif()

add_executable(OrbitQt)

target_sources(
  OrbitQt
  PRIVATE licensedialog.h
          orbitcodeeditor.h
          orbitdataviewpanel.h
          orbitdiffdialog.h
          orbitdisassemblydialog.h
          orbitglwidget.h
          orbitglwidgetwithheader.h
          orbitmainwindow.h
          orbitsamplingreport.h
          orbittablemodel.h
          orbittreeitem.h
          orbittreemodel.h
          orbittreeview.h
          orbitvisualizer.h
          orbitwatchwidget.h
          outputdialog.h
          processlauncherwidget.h
          resource.h
          showincludesdialog.h)

target_sources(
  OrbitQt
  PRIVATE licensedialog.cpp
          main.cpp
          orbitcodeeditor.cpp
          orbitdataviewpanel.cpp
          orbitdiffdialog.cpp
          orbitdisassemblydialog.cpp
          orbitglwidget.cpp
          orbitglwidgetwithheader.cpp
          orbitmainwindow.cpp
          orbitsamplingreport.cpp
          orbittablemodel.cpp
          orbittreeitem.cpp
          orbittreemodel.cpp
          orbittreeview.cpp
          orbitvisualizer.cpp
          orbitwatchwidget.cpp
          outputdialog.cpp
          processlauncherwidget.cpp
          showincludesdialog.cpp)

target_sources(
  OrbitQt
  PRIVATE qtpropertybrowser/qtbuttonpropertybrowser.h
          qtpropertybrowser/qteditorfactory.h
          qtpropertybrowser/qtgroupboxpropertybrowser.h
          qtpropertybrowser/qtpropertybrowser.h
          qtpropertybrowser/qtpropertybrowserutils_p.h
          qtpropertybrowser/qtpropertymanager.h
          qtpropertybrowser/qttreepropertybrowser.h
          qtpropertybrowser/qtvariantproperty.h)
target_sources(
  OrbitQt
  PRIVATE qtpropertybrowser/qtbuttonpropertybrowser.cpp
          qtpropertybrowser/qteditorfactory.cpp
          qtpropertybrowser/qtgroupboxpropertybrowser.cpp
          qtpropertybrowser/qtpropertybrowser.cpp
          qtpropertybrowser/qtpropertybrowserutils.cpp
          qtpropertybrowser/qtpropertymanager.cpp
          qtpropertybrowser/qttreepropertybrowser.cpp
          qtpropertybrowser/qtvariantproperty.cpp)

target_include_directories(OrbitQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitQt PRIVATE OrbitCore OrbitGl Qt5::Widgets Qt5::Core)

if(WIN32)
  # Administrator privileges
  set_target_properties(
    OrbitQt
    PROPERTIES LINK_FLAGS
               "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
endif()

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")
set_target_properties(OrbitQt PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQt PROPERTIES AUTOUIC ON)