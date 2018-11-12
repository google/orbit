set(QTDIR "C:/Qt/5.11.2/msvc2017_64")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QTDIR}")

find_package(Qt5 CONFIG REQUIRED 
	HINTS "${QTDIR}" 
	COMPONENTS Widgets Core
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
    licensedialog.h
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
    showincludesdialog.h
)
set(SOURCES
    licensedialog.cpp
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
    showincludesdialog.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

set(qtpropertybrowser_HEADERS
    qtpropertybrowser/qtbuttonpropertybrowser.h
    qtpropertybrowser/qteditorfactory.h
    qtpropertybrowser/qtgroupboxpropertybrowser.h
    qtpropertybrowser/qtpropertybrowser.h
    qtpropertybrowser/qtpropertybrowserutils_p.h
    qtpropertybrowser/qtpropertymanager.h
    qtpropertybrowser/qttreepropertybrowser.h
    qtpropertybrowser/qtvariantproperty.h
)
set(qtpropertybrowser_SOURCES
    qtpropertybrowser/qtbuttonpropertybrowser.cpp
    qtpropertybrowser/qteditorfactory.cpp
    qtpropertybrowser/qtgroupboxpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowserutils.cpp
    qtpropertybrowser/qtpropertymanager.cpp
    qtpropertybrowser/qttreepropertybrowser.cpp
    qtpropertybrowser/qtvariantproperty.cpp
)
source_group("qtpropertybrowser\\Header Files" FILES ${qtpropertybrowser_HEADERS})
source_group("qtpropertybrowser\\Source Files" FILES ${qtpropertybrowser_SOURCES})

add_executable(OrbitQt 
    ${SOURCES} ${HEADERS} 
	${qtpropertybrowser_SOURCES} ${qtpropertybrowser_HEADERS}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set_target_properties(OrbitQt PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_link_libraries (OrbitQt
	OrbitGl
	OrbitCore
	Qt5::Widgets
	Qt5::Core
	${CURL_LIBRARIES}
	${BREAKPAD_LIB}
)
