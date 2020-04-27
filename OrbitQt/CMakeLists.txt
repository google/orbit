project(OrbitQt CXX)

add_executable(OrbitQt)

target_compile_options(OrbitQt PRIVATE ${STRICT_COMPILE_FLAGS})

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
          OrbitStartupWindow.h
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
          OrbitStartupWindow.cpp
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

target_link_libraries(OrbitQt PRIVATE OrbitCore OrbitGl OrbitGgp Qt5::Widgets Qt5::Core)

if(WIN32)
  # Administrator privileges
  set_target_properties(
    OrbitQt
    PROPERTIES LINK_FLAGS
               "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
  set_target_properties(OrbitQt PROPERTIES WIN32_EXECUTABLE ON)
  target_sources(OrbitQt PRIVATE orbit.rc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(TARGET OrbitQt POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:OrbitQt>"
    COMMENT "Running windeployqt..."
  )
endif()

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")
set_target_properties(OrbitQt PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQt PROPERTIES AUTOUIC ON)
strip_symbols(OrbitQt)

