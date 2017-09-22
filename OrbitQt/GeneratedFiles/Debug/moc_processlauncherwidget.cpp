/****************************************************************************
** Meta object code from reading C++ file 'processlauncherwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../processlauncherwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'processlauncherwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProcessLauncherWidget_t {
    QByteArrayData data[6];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProcessLauncherWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProcessLauncherWidget_t qt_meta_stringdata_ProcessLauncherWidget = {
    {
QT_MOC_LITERAL(0, 0, 21), // "ProcessLauncherWidget"
QT_MOC_LITERAL(1, 22, 23), // "on_BrowseButton_clicked"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 23), // "on_LaunchButton_clicked"
QT_MOC_LITERAL(4, 71, 24), // "on_checkBoxPause_clicked"
QT_MOC_LITERAL(5, 96, 7) // "checked"

    },
    "ProcessLauncherWidget\0on_BrowseButton_clicked\0"
    "\0on_LaunchButton_clicked\0"
    "on_checkBoxPause_clicked\0checked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProcessLauncherWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x08 /* Private */,
       3,    0,   30,    2, 0x08 /* Private */,
       4,    1,   31,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,

       0        // eod
};

void ProcessLauncherWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ProcessLauncherWidget *_t = static_cast<ProcessLauncherWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_BrowseButton_clicked(); break;
        case 1: _t->on_LaunchButton_clicked(); break;
        case 2: _t->on_checkBoxPause_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ProcessLauncherWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ProcessLauncherWidget.data,
      qt_meta_data_ProcessLauncherWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ProcessLauncherWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProcessLauncherWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ProcessLauncherWidget.stringdata0))
        return static_cast<void*>(const_cast< ProcessLauncherWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ProcessLauncherWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
