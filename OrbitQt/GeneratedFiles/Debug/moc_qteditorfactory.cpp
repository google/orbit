/****************************************************************************
** Meta object code from reading C++ file 'qteditorfactory.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qteditorfactory.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qteditorfactory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtSpinBoxFactory_t {
    QByteArrayData data[9];
    char stringdata0[142];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSpinBoxFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSpinBoxFactory_t qt_meta_stringdata_QtSpinBoxFactory = {
    {
QT_MOC_LITERAL(0, 0, 16), // "QtSpinBoxFactory"
QT_MOC_LITERAL(1, 17, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 67, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 89, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(7, 109, 12), // "slotSetValue"
QT_MOC_LITERAL(8, 122, 19) // "slotEditorDestroyed"

    },
    "QtSpinBoxFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotReadOnlyChanged\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSpinBoxFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x08 /* Private */,
       4,    3,   49,    2, 0x08 /* Private */,
       5,    2,   56,    2, 0x08 /* Private */,
       6,    2,   61,    2, 0x08 /* Private */,
       7,    1,   66,    2, 0x08 /* Private */,
       8,    1,   69,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtSpinBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSpinBoxFactory *_t = static_cast<QtSpinBoxFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtSpinBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtSpinBoxFactory.data,
      qt_meta_data_QtSpinBoxFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtSpinBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSpinBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtSpinBoxFactory.stringdata0))
        return static_cast<void*>(const_cast< QtSpinBoxFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtSpinBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
struct qt_meta_stringdata_QtSliderFactory_t {
    QByteArrayData data[8];
    char stringdata0[121];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSliderFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSliderFactory_t qt_meta_stringdata_QtSliderFactory = {
    {
QT_MOC_LITERAL(0, 0, 15), // "QtSliderFactory"
QT_MOC_LITERAL(1, 16, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 49, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 66, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 88, 12), // "slotSetValue"
QT_MOC_LITERAL(7, 101, 19) // "slotEditorDestroyed"

    },
    "QtSliderFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotSetValue\0"
    "slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSliderFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x08 /* Private */,
       4,    3,   44,    2, 0x08 /* Private */,
       5,    2,   51,    2, 0x08 /* Private */,
       6,    1,   56,    2, 0x08 /* Private */,
       7,    1,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtSliderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSliderFactory *_t = static_cast<QtSliderFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtSliderFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtSliderFactory.data,
      qt_meta_data_QtSliderFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtSliderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSliderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtSliderFactory.stringdata0))
        return static_cast<void*>(const_cast< QtSliderFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtSliderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_QtScrollBarFactory_t {
    QByteArrayData data[8];
    char stringdata0[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtScrollBarFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtScrollBarFactory_t qt_meta_stringdata_QtScrollBarFactory = {
    {
QT_MOC_LITERAL(0, 0, 18), // "QtScrollBarFactory"
QT_MOC_LITERAL(1, 19, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 52, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 69, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 91, 12), // "slotSetValue"
QT_MOC_LITERAL(7, 104, 19) // "slotEditorDestroyed"

    },
    "QtScrollBarFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotSetValue\0"
    "slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtScrollBarFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x08 /* Private */,
       4,    3,   44,    2, 0x08 /* Private */,
       5,    2,   51,    2, 0x08 /* Private */,
       6,    1,   56,    2, 0x08 /* Private */,
       7,    1,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtScrollBarFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtScrollBarFactory *_t = static_cast<QtScrollBarFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtScrollBarFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtScrollBarFactory.data,
      qt_meta_data_QtScrollBarFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtScrollBarFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtScrollBarFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtScrollBarFactory.stringdata0))
        return static_cast<void*>(const_cast< QtScrollBarFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtScrollBarFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_QtCheckBoxFactory_t {
    QByteArrayData data[7];
    char stringdata0[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtCheckBoxFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtCheckBoxFactory_t qt_meta_stringdata_QtCheckBoxFactory = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtCheckBoxFactory"
QT_MOC_LITERAL(1, 18, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 51, 22), // "slotTextVisibleChanged"
QT_MOC_LITERAL(5, 74, 12), // "slotSetValue"
QT_MOC_LITERAL(6, 87, 19) // "slotEditorDestroyed"

    },
    "QtCheckBoxFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotTextVisibleChanged\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtCheckBoxFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x08 /* Private */,
       4,    2,   39,    2, 0x08 /* Private */,
       5,    1,   44,    2, 0x08 /* Private */,
       6,    1,   47,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtCheckBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtCheckBoxFactory *_t = static_cast<QtCheckBoxFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotTextVisibleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtCheckBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtBoolPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCheckBoxFactory.data,
      qt_meta_data_QtCheckBoxFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtCheckBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtCheckBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtCheckBoxFactory.stringdata0))
        return static_cast<void*>(const_cast< QtCheckBoxFactory*>(this));
    return QtAbstractEditorFactory<QtBoolPropertyManager>::qt_metacast(_clname);
}

int QtCheckBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtBoolPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_QtDoubleSpinBoxFactory_t {
    QByteArrayData data[10];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDoubleSpinBoxFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDoubleSpinBoxFactory_t qt_meta_stringdata_QtDoubleSpinBoxFactory = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtDoubleSpinBoxFactory"
QT_MOC_LITERAL(1, 23, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 56, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 73, 21), // "slotSingleStepChanged"
QT_MOC_LITERAL(6, 95, 19), // "slotDecimalsChanged"
QT_MOC_LITERAL(7, 115, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(8, 135, 12), // "slotSetValue"
QT_MOC_LITERAL(9, 148, 19) // "slotEditorDestroyed"

    },
    "QtDoubleSpinBoxFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSingleStepChanged\0slotDecimalsChanged\0"
    "slotReadOnlyChanged\0slotSetValue\0"
    "slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDoubleSpinBoxFactory[] = {

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
       1,    2,   49,    2, 0x08 /* Private */,
       4,    3,   54,    2, 0x08 /* Private */,
       5,    2,   61,    2, 0x08 /* Private */,
       6,    2,   66,    2, 0x08 /* Private */,
       7,    2,   71,    2, 0x08 /* Private */,
       8,    1,   76,    2, 0x08 /* Private */,
       9,    1,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtDoubleSpinBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDoubleSpinBoxFactory *_t = static_cast<QtDoubleSpinBoxFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotDecimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotSetValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtDoubleSpinBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDoublePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDoubleSpinBoxFactory.data,
      qt_meta_data_QtDoubleSpinBoxFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDoubleSpinBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDoubleSpinBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDoubleSpinBoxFactory.stringdata0))
        return static_cast<void*>(const_cast< QtDoubleSpinBoxFactory*>(this));
    return QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacast(_clname);
}

int QtDoubleSpinBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtLineEditFactory_t {
    QByteArrayData data[9];
    char stringdata0[142];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtLineEditFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtLineEditFactory_t qt_meta_stringdata_QtLineEditFactory = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtLineEditFactory"
QT_MOC_LITERAL(1, 18, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 51, 17), // "slotRegExpChanged"
QT_MOC_LITERAL(5, 69, 19), // "slotEchoModeChanged"
QT_MOC_LITERAL(6, 89, 19), // "slotReadOnlyChanged"
QT_MOC_LITERAL(7, 109, 12), // "slotSetValue"
QT_MOC_LITERAL(8, 122, 19) // "slotEditorDestroyed"

    },
    "QtLineEditFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRegExpChanged\0"
    "slotEchoModeChanged\0slotReadOnlyChanged\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtLineEditFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x08 /* Private */,
       4,    2,   49,    2, 0x08 /* Private */,
       5,    2,   54,    2, 0x08 /* Private */,
       6,    2,   59,    2, 0x08 /* Private */,
       7,    1,   64,    2, 0x08 /* Private */,
       8,    1,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRegExp,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtLineEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtLineEditFactory *_t = static_cast<QtLineEditFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRegExpChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEchoModeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotReadOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotSetValue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtLineEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtStringPropertyManager>::staticMetaObject, qt_meta_stringdata_QtLineEditFactory.data,
      qt_meta_data_QtLineEditFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtLineEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtLineEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtLineEditFactory.stringdata0))
        return static_cast<void*>(const_cast< QtLineEditFactory*>(this));
    return QtAbstractEditorFactory<QtStringPropertyManager>::qt_metacast(_clname);
}

int QtLineEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtStringPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
struct qt_meta_stringdata_QtDateEditFactory_t {
    QByteArrayData data[7];
    char stringdata0[101];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDateEditFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDateEditFactory_t qt_meta_stringdata_QtDateEditFactory = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtDateEditFactory"
QT_MOC_LITERAL(1, 18, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 51, 16), // "slotRangeChanged"
QT_MOC_LITERAL(5, 68, 12), // "slotSetValue"
QT_MOC_LITERAL(6, 81, 19) // "slotEditorDestroyed"

    },
    "QtDateEditFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotRangeChanged\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDateEditFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x08 /* Private */,
       4,    3,   39,    2, 0x08 /* Private */,
       5,    1,   46,    2, 0x08 /* Private */,
       6,    1,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate, QMetaType::QDate,    2,    2,    2,
    QMetaType::Void, QMetaType::QDate,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtDateEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDateEditFactory *_t = static_cast<QtDateEditFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QDate(*)>(_a[1]))); break;
        case 3: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtDateEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDatePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDateEditFactory.data,
      qt_meta_data_QtDateEditFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDateEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDateEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateEditFactory.stringdata0))
        return static_cast<void*>(const_cast< QtDateEditFactory*>(this));
    return QtAbstractEditorFactory<QtDatePropertyManager>::qt_metacast(_clname);
}

int QtDateEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDatePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_QtTimeEditFactory_t {
    QByteArrayData data[6];
    char stringdata0[84];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtTimeEditFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtTimeEditFactory_t qt_meta_stringdata_QtTimeEditFactory = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtTimeEditFactory"
QT_MOC_LITERAL(1, 18, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 51, 12), // "slotSetValue"
QT_MOC_LITERAL(5, 64, 19) // "slotEditorDestroyed"

    },
    "QtTimeEditFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtTimeEditFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QTime,    2,    2,
    QMetaType::Void, QMetaType::QTime,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtTimeEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtTimeEditFactory *_t = static_cast<QtTimeEditFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QTime(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtTimeEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtTimePropertyManager>::staticMetaObject, qt_meta_stringdata_QtTimeEditFactory.data,
      qt_meta_data_QtTimeEditFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtTimeEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtTimeEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtTimeEditFactory.stringdata0))
        return static_cast<void*>(const_cast< QtTimeEditFactory*>(this));
    return QtAbstractEditorFactory<QtTimePropertyManager>::qt_metacast(_clname);
}

int QtTimeEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtTimePropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtDateTimeEditFactory_t {
    QByteArrayData data[6];
    char stringdata0[88];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDateTimeEditFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDateTimeEditFactory_t qt_meta_stringdata_QtDateTimeEditFactory = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtDateTimeEditFactory"
QT_MOC_LITERAL(1, 22, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 55, 12), // "slotSetValue"
QT_MOC_LITERAL(5, 68, 19) // "slotEditorDestroyed"

    },
    "QtDateTimeEditFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDateTimeEditFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDateTime,    2,    2,
    QMetaType::Void, QMetaType::QDateTime,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtDateTimeEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDateTimeEditFactory *_t = static_cast<QtDateTimeEditFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtDateTimeEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDateTimePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDateTimeEditFactory.data,
      qt_meta_data_QtDateTimeEditFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDateTimeEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDateTimeEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateTimeEditFactory.stringdata0))
        return static_cast<void*>(const_cast< QtDateTimeEditFactory*>(this));
    return QtAbstractEditorFactory<QtDateTimePropertyManager>::qt_metacast(_clname);
}

int QtDateTimeEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDateTimePropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtKeySequenceEditorFactory_t {
    QByteArrayData data[6];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtKeySequenceEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtKeySequenceEditorFactory_t qt_meta_stringdata_QtKeySequenceEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 26), // "QtKeySequenceEditorFactory"
QT_MOC_LITERAL(1, 27, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 60, 12), // "slotSetValue"
QT_MOC_LITERAL(5, 73, 19) // "slotEditorDestroyed"

    },
    "QtKeySequenceEditorFactory\0"
    "slotPropertyChanged\0\0QtProperty*\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtKeySequenceEditorFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QKeySequence,    2,    2,
    QMetaType::Void, QMetaType::QKeySequence,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtKeySequenceEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtKeySequenceEditorFactory *_t = static_cast<QtKeySequenceEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QKeySequence(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtKeySequenceEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtKeySequencePropertyManager>::staticMetaObject, qt_meta_stringdata_QtKeySequenceEditorFactory.data,
      qt_meta_data_QtKeySequenceEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtKeySequenceEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtKeySequenceEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtKeySequenceEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtKeySequenceEditorFactory*>(this));
    return QtAbstractEditorFactory<QtKeySequencePropertyManager>::qt_metacast(_clname);
}

int QtKeySequenceEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtKeySequencePropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtCharEditorFactory_t {
    QByteArrayData data[6];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtCharEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtCharEditorFactory_t qt_meta_stringdata_QtCharEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 19), // "QtCharEditorFactory"
QT_MOC_LITERAL(1, 20, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 53, 12), // "slotSetValue"
QT_MOC_LITERAL(5, 66, 19) // "slotEditorDestroyed"

    },
    "QtCharEditorFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtCharEditorFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QChar,    2,    2,
    QMetaType::Void, QMetaType::QChar,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtCharEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtCharEditorFactory *_t = static_cast<QtCharEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QChar(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtCharEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtCharPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCharEditorFactory.data,
      qt_meta_data_QtCharEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtCharEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtCharEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtCharEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtCharEditorFactory*>(this));
    return QtAbstractEditorFactory<QtCharPropertyManager>::qt_metacast(_clname);
}

int QtCharEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtCharPropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtEnumEditorFactory_t {
    QByteArrayData data[9];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtEnumEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtEnumEditorFactory_t qt_meta_stringdata_QtEnumEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 19), // "QtEnumEditorFactory"
QT_MOC_LITERAL(1, 20, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 53, 20), // "slotEnumNamesChanged"
QT_MOC_LITERAL(5, 74, 20), // "slotEnumIconsChanged"
QT_MOC_LITERAL(6, 95, 15), // "QMap<int,QIcon>"
QT_MOC_LITERAL(7, 111, 12), // "slotSetValue"
QT_MOC_LITERAL(8, 124, 19) // "slotEditorDestroyed"

    },
    "QtEnumEditorFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotEnumNamesChanged\0"
    "slotEnumIconsChanged\0QMap<int,QIcon>\0"
    "slotSetValue\0slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtEnumEditorFactory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x08 /* Private */,
       4,    2,   44,    2, 0x08 /* Private */,
       5,    2,   49,    2, 0x08 /* Private */,
       7,    1,   54,    2, 0x08 /* Private */,
       8,    1,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    2,    2,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 6,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtEnumEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtEnumEditorFactory *_t = static_cast<QtEnumEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEnumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEnumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtEnumEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtEnumPropertyManager>::staticMetaObject, qt_meta_stringdata_QtEnumEditorFactory.data,
      qt_meta_data_QtEnumEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtEnumEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtEnumEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtEnumEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtEnumEditorFactory*>(this));
    return QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacast(_clname);
}

int QtEnumEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_QtCursorEditorFactory_t {
    QByteArrayData data[6];
    char stringdata0[91];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtCursorEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtCursorEditorFactory_t qt_meta_stringdata_QtCursorEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtCursorEditorFactory"
QT_MOC_LITERAL(1, 22, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 55, 15), // "slotEnumChanged"
QT_MOC_LITERAL(5, 71, 19) // "slotEditorDestroyed"

    },
    "QtCursorEditorFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotEnumChanged\0"
    "slotEditorDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtCursorEditorFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    2,   34,    2, 0x08 /* Private */,
       5,    1,   39,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QCursor,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,

       0        // eod
};

void QtCursorEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtCursorEditorFactory *_t = static_cast<QtCursorEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtCursorEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtCursorPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCursorEditorFactory.data,
      qt_meta_data_QtCursorEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtCursorEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtCursorEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtCursorEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtCursorEditorFactory*>(this));
    return QtAbstractEditorFactory<QtCursorPropertyManager>::qt_metacast(_clname);
}

int QtCursorEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtCursorPropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtColorEditorFactory_t {
    QByteArrayData data[6];
    char stringdata0[87];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtColorEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtColorEditorFactory_t qt_meta_stringdata_QtColorEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 20), // "QtColorEditorFactory"
QT_MOC_LITERAL(1, 21, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 54, 19), // "slotEditorDestroyed"
QT_MOC_LITERAL(5, 74, 12) // "slotSetValue"

    },
    "QtColorEditorFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotEditorDestroyed\0"
    "slotSetValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtColorEditorFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QColor,    2,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,
    QMetaType::Void, QMetaType::QColor,    2,

       0        // eod
};

void QtColorEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtColorEditorFactory *_t = static_cast<QtColorEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtColorEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtColorPropertyManager>::staticMetaObject, qt_meta_stringdata_QtColorEditorFactory.data,
      qt_meta_data_QtColorEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtColorEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtColorEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtColorEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtColorEditorFactory*>(this));
    return QtAbstractEditorFactory<QtColorPropertyManager>::qt_metacast(_clname);
}

int QtColorEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtColorPropertyManager>::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_QtFontEditorFactory_t {
    QByteArrayData data[6];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtFontEditorFactory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtFontEditorFactory_t qt_meta_stringdata_QtFontEditorFactory = {
    {
QT_MOC_LITERAL(0, 0, 19), // "QtFontEditorFactory"
QT_MOC_LITERAL(1, 20, 19), // "slotPropertyChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 53, 19), // "slotEditorDestroyed"
QT_MOC_LITERAL(5, 73, 12) // "slotSetValue"

    },
    "QtFontEditorFactory\0slotPropertyChanged\0"
    "\0QtProperty*\0slotEditorDestroyed\0"
    "slotSetValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFontEditorFactory[] = {

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
       1,    2,   29,    2, 0x08 /* Private */,
       4,    1,   34,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QFont,    2,    2,
    QMetaType::Void, QMetaType::QObjectStar,    2,
    QMetaType::Void, QMetaType::QFont,    2,

       0        // eod
};

void QtFontEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtFontEditorFactory *_t = static_cast<QtFontEditorFactory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtFontEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtFontPropertyManager>::staticMetaObject, qt_meta_stringdata_QtFontEditorFactory.data,
      qt_meta_data_QtFontEditorFactory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtFontEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtFontEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtFontEditorFactory.stringdata0))
        return static_cast<void*>(const_cast< QtFontEditorFactory*>(this));
    return QtAbstractEditorFactory<QtFontPropertyManager>::qt_metacast(_clname);
}

int QtFontEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtFontPropertyManager>::qt_metacall(_c, _id, _a);
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
