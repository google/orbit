project(OrbitQt CXX)

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

# Hack: Copying files out of the build-directory should really not be necessary.
# freetype-gl should install these files. But for now, that's the best
# workaround.
set(FREETYPE_GL_BUILDTREE
    "${CMAKE_CURRENT_LIST_DIR}/../external/vcpkg/buildtrees/freetype-gl/src/014d3f81b8-2461f3be10"
)

add_custom_command(
  TARGET OrbitQt
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "$<TARGET_FILE_DIR:OrbitQt>/shaders")

add_custom_command(
  TARGET OrbitQt
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${FREETYPE_GL_BUILDTREE}/shaders/v3f-t2f-c4f.frag"
    "${FREETYPE_GL_BUILDTREE}/shaders/v3f-t2f-c4f.vert"
    "$<TARGET_FILE_DIR:OrbitQt>/shaders/")

add_custom_command(
  TARGET OrbitQt
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OrbitQt>/fonts")

add_custom_command(
  TARGET OrbitQt
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${FREETYPE_GL_BUILDTREE}/fonts/Vera.ttf"
    "$<TARGET_FILE_DIR:OrbitQt>/fonts/Vera.ttf")