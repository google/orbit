#-------------------------------------------------
#
# Project created by QtCreator 2016-09-15T10:33:33
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = orbit
TEMPLATE = app

DEFINES += ORBIT_PROFILER \
           UNICODE

INCLUDEPATH += $$PWD/../

SOURCES += main.cpp \
           licensedialog.cpp \
           main.cpp \
           orbitcodeeditor.cpp \
           orbitdataviewpanel.cpp \
           orbitglwidget.cpp \
           orbitglwidgetwithheader.cpp \
           orbitmainwindow.cpp \
           orbitsamplingreport.cpp \
           orbittablemodel.cpp \
           orbittreeview.cpp \
           outputdialog.cpp \
           processlauncherwidget.cpp \
           showincludesdialog.cpp \
           orbittreeitem.cpp \
           orbittreemodel.cpp \
           orbitdiffdialog.cpp \
           qtpropertybrowser/qtbuttonpropertybrowser.cpp \
           qtpropertybrowser/qteditorfactory.cpp \
           qtpropertybrowser/qtgroupboxpropertybrowser.cpp \
           qtpropertybrowser/qtpropertybrowser.cpp \
           qtpropertybrowser/qtpropertybrowserutils.cpp \
           qtpropertybrowser/qtpropertymanager.cpp \
           qtpropertybrowser/qttreepropertybrowser.cpp \
           qtpropertybrowser/qtvariantproperty.cpp \
           orbitwatchwidget.cpp \
           orbitdisassemblydialog.cpp \
           orbitvisualizer.cpp

HEADERS  += \
    orbitmainwindow.h \
    orbitglwidget.h \
    orbittreeview.h \
    orbittablemodel.h \
    orbitdataviewpanel.h \
    orbitsamplingreport.h \
    orbitglwidgetwithheader.h \
    orbitcodeeditor.h \
    licensedialog.h \
    outputdialog.h \
    processlauncherwidget.h \
    showincludesdialog.h \
    orbittreeitem.h \
    orbittreemodel.h \
    orbitdiffdialog.h \
    qtpropertybrowser/qtbuttonpropertybrowser.h \
    qtpropertybrowser/qteditorfactory.h \
    qtpropertybrowser/qtgroupboxpropertybrowser.h \
    qtpropertybrowser/qtpropertybrowser.h \
    qtpropertybrowser/qtpropertybrowserutils_p.h \
    qtpropertybrowser/qtpropertymanager.h \
    qtpropertybrowser/qttreepropertybrowser.h \
    qtpropertybrowser/qtvariantproperty.h \
    orbitwatchwidget.h \
    orbitdisassemblydialog.h \
    orbitvisualizer.h

FORMS    += \
    orbitmainwindow.ui \
    orbitdataviewpanel.ui \
    orbitsamplingreport.ui \
    orbitglwidgetwithheader.ui \
    licensedialog.ui \
    outputdialog.ui \
    processlauncherwidget.ui \
    showincludesdialog.ui \
    orbitdiffdialog.ui \
    orbitwatchwidget.ui \
    orbitdisassemblydialog.ui \
    orbitvisualizer.ui

RC_FILE  += \
    OrbitQt.rc

DISTFILES += \
    orbit_16_32_48_256.ico \
    qtpropertybrowser/qtpropertybrowser.pri

INCLUDEPATH += \
    ../external/xxHash-r42/ \
    ../external/imgui/

LIBS += -L$$PWD/../external/curl-7.52.1/lib/                  -llibcurl_imp
LIBS += -L$$PWD/../external/glew-2.0.0/lib/Release/x64/       -lglew32
LIBS +=                                                       -lopengl32
LIBS +=                                                       -lglu32
LIBS +=                                                       -dbghelp
LIBS += -L$$PWD/../external/freetype-gl/x64/Release/          -lfreetype-gl
LIBS += -L$$PWD/../external/freetype/lib/x64/                 -lfreetype271

CONFIG( debug, debug|release ) {
    # debug
    LIBS += -L$$PWD/../bin/x64/debug/                                      -lOrbitCore
    LIBS += -L$$PWD/../bin/x64/debug/                                      -lOrbitGl
    LIBS += -L$$PWD/../bin/x64/debug/                                      -lOrbitAsm
    LIBS += -L$$PWD/../external/freeglut-2.8.1/lib/x64/debug/              -lfreeglut_static
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Debug/lib/ -lexception_handler
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Debug/lib/ -lcrash_generation_client
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Debug/lib/ -lcommon
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Debug/lib/ -lexception_handler
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Debug/lib/ -lprocessor_bits
    LIBS += -L$$PWD/../external/capstone/msvc/x64/Release/                 -lcapstone

    OBJECTS_DIR = $$PWD/../intermediate/x64/OrbitQt/debug/
    DESTDIR     = $$PWD/../bin/x64/debug/
    UI_DIR      = $$PWD/GeneratedFiles/debug/
    MOC_DIR     = $$PWD/GeneratedFiles/debug/
} else {
    # release
    LIBS += -L$$PWD/../bin/x64/release/                                      -lOrbitCore
    LIBS += -L$$PWD/../bin/x64/release/                                      -lOrbitGl
    LIBS += -L$$PWD/../bin/x64/release/                                      -lOrbitAsm
    LIBS += -L$$PWD/../external/freeglut-2.8.1/lib/x64/                      -lfreeglut_static
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Release/lib/ -lexception_handler
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Release/lib/ -lcrash_generation_client
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Release/lib/ -lcommon
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Release/lib/ -lexception_handler
    LIBS += -L$$PWD/../external/breakpad/src/client/windows/x64/Release/lib/ -lprocessor_bits
    LIBS += -L$$PWD/../external/capstone/msvc/x64/Release/                   -lcapstone

    OBJECTS_DIR = $$PWD/../intermediate/x64/OrbitQt/release/
    DESTDIR     = $$PWD/../bin/x64/release/
    UI_DIR      = $$PWD/GeneratedFiles/release/
    MOC_DIR     = $$PWD/GeneratedFiles/release/
}

CONFIG += embed_manifest_exe
QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

QMAKE_LFLAGS_RELEASE += /MAP
QMAKE_CFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /debug /opt:ref

RESOURCES += \
    qtpropertybrowser/qtpropertybrowser.qrc

