/****************************************************************************
** Meta object code from reading C++ file 'orbitwatchwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../orbitwatchwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'orbitwatchwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OrbitWatchWidget_t {
    QByteArrayData data[10];
    char stringdata0[137];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OrbitWatchWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OrbitWatchWidget_t qt_meta_stringdata_OrbitWatchWidget = {
    {
QT_MOC_LITERAL(0, 0, 16), // "OrbitWatchWidget"
QT_MOC_LITERAL(1, 17, 27), // "on_FindLineEdit_textChanged"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 4), // "arg1"
QT_MOC_LITERAL(4, 51, 12), // "valueChanged"
QT_MOC_LITERAL(5, 64, 11), // "QtProperty*"
QT_MOC_LITERAL(6, 76, 8), // "property"
QT_MOC_LITERAL(7, 85, 3), // "val"
QT_MOC_LITERAL(8, 89, 24), // "on_RefreshButton_clicked"
QT_MOC_LITERAL(9, 114, 22) // "on_ClearButton_clicked"

    },
    "OrbitWatchWidget\0on_FindLineEdit_textChanged\0"
    "\0arg1\0valueChanged\0QtProperty*\0property\0"
    "val\0on_RefreshButton_clicked\0"
    "on_ClearButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OrbitWatchWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x08 /* Private */,
       4,    2,   52,    2, 0x08 /* Private */,
       4,    2,   57,    2, 0x08 /* Private */,
       4,    2,   62,    2, 0x08 /* Private */,
       4,    2,   67,    2, 0x08 /* Private */,
       8,    0,   72,    2, 0x08 /* Private */,
       9,    0,   73,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    6,    7,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Bool,    6,    7,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Double,    6,    7,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QString,    6,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void OrbitWatchWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OrbitWatchWidget *_t = static_cast<OrbitWatchWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_FindLineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 4: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: _t->on_RefreshButton_clicked(); break;
        case 6: _t->on_ClearButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject OrbitWatchWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_OrbitWatchWidget.data,
      qt_meta_data_OrbitWatchWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *OrbitWatchWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrbitWatchWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_OrbitWatchWidget.stringdata0))
        return static_cast<void*>(const_cast< OrbitWatchWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int OrbitWatchWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
