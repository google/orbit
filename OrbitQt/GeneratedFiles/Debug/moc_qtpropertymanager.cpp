/****************************************************************************
** Meta object code from reading C++ file 'qtpropertymanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qtpropertymanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtpropertymanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtGroupPropertyManager_t {
    QByteArrayData data[1];
    char stringdata0[23];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtGroupPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtGroupPropertyManager_t qt_meta_stringdata_QtGroupPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 22) // "QtGroupPropertyManager"

    },
    "QtGroupPropertyManager"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtGroupPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void QtGroupPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject QtGroupPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtGroupPropertyManager.data,
      qt_meta_data_QtGroupPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtGroupPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtGroupPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtGroupPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtGroupPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtGroupPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_QtIntPropertyManager_t {
    QByteArrayData data[19];
    char stringdata0[201];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtIntPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtIntPropertyManager_t qt_meta_stringdata_QtIntPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 20), // "QtIntPropertyManager"
QT_MOC_LITERAL(1, 21, 12), // "valueChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 47, 8), // "property"
QT_MOC_LITERAL(5, 56, 3), // "val"
QT_MOC_LITERAL(6, 60, 12), // "rangeChanged"
QT_MOC_LITERAL(7, 73, 6), // "minVal"
QT_MOC_LITERAL(8, 80, 6), // "maxVal"
QT_MOC_LITERAL(9, 87, 17), // "singleStepChanged"
QT_MOC_LITERAL(10, 105, 4), // "step"
QT_MOC_LITERAL(11, 110, 15), // "readOnlyChanged"
QT_MOC_LITERAL(12, 126, 8), // "readOnly"
QT_MOC_LITERAL(13, 135, 8), // "setValue"
QT_MOC_LITERAL(14, 144, 10), // "setMinimum"
QT_MOC_LITERAL(15, 155, 10), // "setMaximum"
QT_MOC_LITERAL(16, 166, 8), // "setRange"
QT_MOC_LITERAL(17, 175, 13), // "setSingleStep"
QT_MOC_LITERAL(18, 189, 11) // "setReadOnly"

    },
    "QtIntPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0rangeChanged\0"
    "minVal\0maxVal\0singleStepChanged\0step\0"
    "readOnlyChanged\0readOnly\0setValue\0"
    "setMinimum\0setMaximum\0setRange\0"
    "setSingleStep\0setReadOnly"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtIntPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       6,    3,   69,    2, 0x06 /* Public */,
       9,    2,   76,    2, 0x06 /* Public */,
      11,    2,   81,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    2,   86,    2, 0x0a /* Public */,
      14,    2,   91,    2, 0x0a /* Public */,
      15,    2,   96,    2, 0x0a /* Public */,
      16,    3,  101,    2, 0x0a /* Public */,
      17,    2,  108,    2, 0x0a /* Public */,
      18,    2,  113,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   12,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   12,

       0        // eod
};

void QtIntPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtIntPropertyManager *_t = static_cast<QtIntPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->singleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->readOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 8: _t->setSingleStep((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->setReadOnly((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtIntPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtIntPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtIntPropertyManager::*_t)(QtProperty * , int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtIntPropertyManager::rangeChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtIntPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtIntPropertyManager::singleStepChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (QtIntPropertyManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtIntPropertyManager::readOnlyChanged)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject QtIntPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtIntPropertyManager.data,
      qt_meta_data_QtIntPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtIntPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtIntPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtIntPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtIntPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtIntPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QtIntPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtIntPropertyManager::rangeChanged(QtProperty * _t1, int _t2, int _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtIntPropertyManager::singleStepChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtIntPropertyManager::readOnlyChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
struct qt_meta_stringdata_QtBoolPropertyManager_t {
    QByteArrayData data[10];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtBoolPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtBoolPropertyManager_t qt_meta_stringdata_QtBoolPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtBoolPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 18), // "textVisibleChanged"
QT_MOC_LITERAL(7, 80, 8), // "setValue"
QT_MOC_LITERAL(8, 89, 14), // "setTextVisible"
QT_MOC_LITERAL(9, 104, 11) // "textVisible"

    },
    "QtBoolPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0textVisibleChanged\0"
    "setValue\0setTextVisible\0textVisible"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtBoolPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,
       6,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   44,    2, 0x0a /* Public */,
       8,    2,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    9,

       0        // eod
};

void QtBoolPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtBoolPropertyManager *_t = static_cast<QtBoolPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->textVisibleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->setTextVisible((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtBoolPropertyManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtBoolPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtBoolPropertyManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtBoolPropertyManager::textVisibleChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtBoolPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtBoolPropertyManager.data,
      qt_meta_data_QtBoolPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtBoolPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtBoolPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtBoolPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtBoolPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtBoolPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtBoolPropertyManager::valueChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtBoolPropertyManager::textVisibleChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtDoublePropertyManager_t {
    QByteArrayData data[22];
    char stringdata0[237];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDoublePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDoublePropertyManager_t qt_meta_stringdata_QtDoublePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtDoublePropertyManager"
QT_MOC_LITERAL(1, 24, 12), // "valueChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 8), // "property"
QT_MOC_LITERAL(5, 59, 3), // "val"
QT_MOC_LITERAL(6, 63, 12), // "rangeChanged"
QT_MOC_LITERAL(7, 76, 6), // "minVal"
QT_MOC_LITERAL(8, 83, 6), // "maxVal"
QT_MOC_LITERAL(9, 90, 17), // "singleStepChanged"
QT_MOC_LITERAL(10, 108, 4), // "step"
QT_MOC_LITERAL(11, 113, 15), // "decimalsChanged"
QT_MOC_LITERAL(12, 129, 4), // "prec"
QT_MOC_LITERAL(13, 134, 15), // "readOnlyChanged"
QT_MOC_LITERAL(14, 150, 8), // "readOnly"
QT_MOC_LITERAL(15, 159, 8), // "setValue"
QT_MOC_LITERAL(16, 168, 10), // "setMinimum"
QT_MOC_LITERAL(17, 179, 10), // "setMaximum"
QT_MOC_LITERAL(18, 190, 8), // "setRange"
QT_MOC_LITERAL(19, 199, 13), // "setSingleStep"
QT_MOC_LITERAL(20, 213, 11), // "setDecimals"
QT_MOC_LITERAL(21, 225, 11) // "setReadOnly"

    },
    "QtDoublePropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0rangeChanged\0"
    "minVal\0maxVal\0singleStepChanged\0step\0"
    "decimalsChanged\0prec\0readOnlyChanged\0"
    "readOnly\0setValue\0setMinimum\0setMaximum\0"
    "setRange\0setSingleStep\0setDecimals\0"
    "setReadOnly"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDoublePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,
       6,    3,   79,    2, 0x06 /* Public */,
       9,    2,   86,    2, 0x06 /* Public */,
      11,    2,   91,    2, 0x06 /* Public */,
      13,    2,   96,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    2,  101,    2, 0x0a /* Public */,
      16,    2,  106,    2, 0x0a /* Public */,
      17,    2,  111,    2, 0x0a /* Public */,
      18,    3,  116,    2, 0x0a /* Public */,
      19,    2,  123,    2, 0x0a /* Public */,
      20,    2,  128,    2, 0x0a /* Public */,
      21,    2,  133,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   12,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   14,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   12,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   14,

       0        // eod
};

void QtDoublePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDoublePropertyManager *_t = static_cast<QtDoublePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->singleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->readOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 6: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 7: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 8: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 9: _t->setSingleStep((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 10: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: _t->setReadOnly((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtDoublePropertyManager::*_t)(QtProperty * , double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDoublePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtDoublePropertyManager::*_t)(QtProperty * , double , double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDoublePropertyManager::rangeChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtDoublePropertyManager::*_t)(QtProperty * , double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDoublePropertyManager::singleStepChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (QtDoublePropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDoublePropertyManager::decimalsChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (QtDoublePropertyManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDoublePropertyManager::readOnlyChanged)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject QtDoublePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDoublePropertyManager.data,
      qt_meta_data_QtDoublePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDoublePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDoublePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDoublePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtDoublePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDoublePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void QtDoublePropertyManager::valueChanged(QtProperty * _t1, double _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDoublePropertyManager::rangeChanged(QtProperty * _t1, double _t2, double _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDoublePropertyManager::singleStepChanged(QtProperty * _t1, double _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtDoublePropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QtDoublePropertyManager::readOnlyChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
struct qt_meta_stringdata_QtStringPropertyManager_t {
    QByteArrayData data[17];
    char stringdata0[186];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtStringPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtStringPropertyManager_t qt_meta_stringdata_QtStringPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtStringPropertyManager"
QT_MOC_LITERAL(1, 24, 12), // "valueChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 8), // "property"
QT_MOC_LITERAL(5, 59, 3), // "val"
QT_MOC_LITERAL(6, 63, 13), // "regExpChanged"
QT_MOC_LITERAL(7, 77, 6), // "regExp"
QT_MOC_LITERAL(8, 84, 15), // "echoModeChanged"
QT_MOC_LITERAL(9, 100, 15), // "readOnlyChanged"
QT_MOC_LITERAL(10, 116, 8), // "setValue"
QT_MOC_LITERAL(11, 125, 9), // "setRegExp"
QT_MOC_LITERAL(12, 135, 11), // "setEchoMode"
QT_MOC_LITERAL(13, 147, 8), // "EchoMode"
QT_MOC_LITERAL(14, 156, 8), // "echoMode"
QT_MOC_LITERAL(15, 165, 11), // "setReadOnly"
QT_MOC_LITERAL(16, 177, 8) // "readOnly"

    },
    "QtStringPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0regExpChanged\0"
    "regExp\0echoModeChanged\0readOnlyChanged\0"
    "setValue\0setRegExp\0setEchoMode\0EchoMode\0"
    "echoMode\0setReadOnly\0readOnly"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtStringPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   54,    2, 0x06 /* Public */,
       6,    2,   59,    2, 0x06 /* Public */,
       8,    2,   64,    2, 0x06 /* Public */,
       9,    2,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    2,   74,    2, 0x0a /* Public */,
      11,    2,   79,    2, 0x0a /* Public */,
      12,    2,   84,    2, 0x0a /* Public */,
      15,    2,   89,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRegExp,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRegExp,    4,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 13,    4,   14,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,   16,

       0        // eod
};

void QtStringPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtStringPropertyManager *_t = static_cast<QtStringPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->regExpChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 2: _t->echoModeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 3: _t->readOnlyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: _t->setRegExp((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 6: _t->setEchoMode((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< EchoMode(*)>(_a[2]))); break;
        case 7: _t->setReadOnly((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtStringPropertyManager::*_t)(QtProperty * , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtStringPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtStringPropertyManager::*_t)(QtProperty * , const QRegExp & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtStringPropertyManager::regExpChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtStringPropertyManager::*_t)(QtProperty * , const int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtStringPropertyManager::echoModeChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (QtStringPropertyManager::*_t)(QtProperty * , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtStringPropertyManager::readOnlyChanged)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject QtStringPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtStringPropertyManager.data,
      qt_meta_data_QtStringPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtStringPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtStringPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtStringPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtStringPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtStringPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtStringPropertyManager::valueChanged(QtProperty * _t1, const QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtStringPropertyManager::regExpChanged(QtProperty * _t1, const QRegExp & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtStringPropertyManager::echoModeChanged(QtProperty * _t1, const int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtStringPropertyManager::readOnlyChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
struct qt_meta_stringdata_QtDatePropertyManager_t {
    QByteArrayData data[13];
    char stringdata0[128];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDatePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDatePropertyManager_t qt_meta_stringdata_QtDatePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtDatePropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 12), // "rangeChanged"
QT_MOC_LITERAL(7, 74, 6), // "minVal"
QT_MOC_LITERAL(8, 81, 6), // "maxVal"
QT_MOC_LITERAL(9, 88, 8), // "setValue"
QT_MOC_LITERAL(10, 97, 10), // "setMinimum"
QT_MOC_LITERAL(11, 108, 10), // "setMaximum"
QT_MOC_LITERAL(12, 119, 8) // "setRange"

    },
    "QtDatePropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0rangeChanged\0"
    "minVal\0maxVal\0setValue\0setMinimum\0"
    "setMaximum\0setRange"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDatePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    3,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    2,   56,    2, 0x0a /* Public */,
      10,    2,   61,    2, 0x0a /* Public */,
      11,    2,   66,    2, 0x0a /* Public */,
      12,    3,   71,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate, QMetaType::QDate,    4,    7,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate,    4,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDate, QMetaType::QDate,    4,    7,    8,

       0        // eod
};

void QtDatePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDatePropertyManager *_t = static_cast<QtDatePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 3: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 4: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 5: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtDatePropertyManager::*_t)(QtProperty * , const QDate & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDatePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtDatePropertyManager::*_t)(QtProperty * , const QDate & , const QDate & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDatePropertyManager::rangeChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtDatePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDatePropertyManager.data,
      qt_meta_data_QtDatePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDatePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDatePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDatePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtDatePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDatePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtDatePropertyManager::valueChanged(QtProperty * _t1, const QDate & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDatePropertyManager::rangeChanged(QtProperty * _t1, const QDate & _t2, const QDate & _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtTimePropertyManager_t {
    QByteArrayData data[7];
    char stringdata0[70];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtTimePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtTimePropertyManager_t qt_meta_stringdata_QtTimePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtTimePropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 8) // "setValue"

    },
    "QtTimePropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0setValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtTimePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QTime,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QTime,    4,    5,

       0        // eod
};

void QtTimePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtTimePropertyManager *_t = static_cast<QtTimePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtTimePropertyManager::*_t)(QtProperty * , const QTime & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtTimePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtTimePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtTimePropertyManager.data,
      qt_meta_data_QtTimePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtTimePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtTimePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtTimePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtTimePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtTimePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtTimePropertyManager::valueChanged(QtProperty * _t1, const QTime & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtDateTimePropertyManager_t {
    QByteArrayData data[7];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDateTimePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDateTimePropertyManager_t qt_meta_stringdata_QtDateTimePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 25), // "QtDateTimePropertyManager"
QT_MOC_LITERAL(1, 26, 12), // "valueChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 52, 8), // "property"
QT_MOC_LITERAL(5, 61, 3), // "val"
QT_MOC_LITERAL(6, 65, 8) // "setValue"

    },
    "QtDateTimePropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDateTimePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDateTime,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QDateTime,    4,    5,

       0        // eod
};

void QtDateTimePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtDateTimePropertyManager *_t = static_cast<QtDateTimePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtDateTimePropertyManager::*_t)(QtProperty * , const QDateTime & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtDateTimePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtDateTimePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDateTimePropertyManager.data,
      qt_meta_data_QtDateTimePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtDateTimePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDateTimePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateTimePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtDateTimePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDateTimePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtDateTimePropertyManager::valueChanged(QtProperty * _t1, const QDateTime & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtKeySequencePropertyManager_t {
    QByteArrayData data[7];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtKeySequencePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtKeySequencePropertyManager_t qt_meta_stringdata_QtKeySequencePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 28), // "QtKeySequencePropertyManager"
QT_MOC_LITERAL(1, 29, 12), // "valueChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 55, 8), // "property"
QT_MOC_LITERAL(5, 64, 3), // "val"
QT_MOC_LITERAL(6, 68, 8) // "setValue"

    },
    "QtKeySequencePropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtKeySequencePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QKeySequence,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QKeySequence,    4,    5,

       0        // eod
};

void QtKeySequencePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtKeySequencePropertyManager *_t = static_cast<QtKeySequencePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtKeySequencePropertyManager::*_t)(QtProperty * , const QKeySequence & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtKeySequencePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtKeySequencePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtKeySequencePropertyManager.data,
      qt_meta_data_QtKeySequencePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtKeySequencePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtKeySequencePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtKeySequencePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtKeySequencePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtKeySequencePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtKeySequencePropertyManager::valueChanged(QtProperty * _t1, const QKeySequence & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtCharPropertyManager_t {
    QByteArrayData data[7];
    char stringdata0[70];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtCharPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtCharPropertyManager_t qt_meta_stringdata_QtCharPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtCharPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 8) // "setValue"

    },
    "QtCharPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0setValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtCharPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QChar,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QChar,    4,    5,

       0        // eod
};

void QtCharPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtCharPropertyManager *_t = static_cast<QtCharPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtCharPropertyManager::*_t)(QtProperty * , const QChar & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtCharPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtCharPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtCharPropertyManager.data,
      qt_meta_data_QtCharPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtCharPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtCharPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtCharPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtCharPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtCharPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtCharPropertyManager::valueChanged(QtProperty * _t1, const QChar & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtLocalePropertyManager_t {
    QByteArrayData data[9];
    char stringdata0[110];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtLocalePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtLocalePropertyManager_t qt_meta_stringdata_QtLocalePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtLocalePropertyManager"
QT_MOC_LITERAL(1, 24, 12), // "valueChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 8), // "property"
QT_MOC_LITERAL(5, 59, 3), // "val"
QT_MOC_LITERAL(6, 63, 8), // "setValue"
QT_MOC_LITERAL(7, 72, 15), // "slotEnumChanged"
QT_MOC_LITERAL(8, 88, 21) // "slotPropertyDestroyed"

    },
    "QtLocalePropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue\0"
    "slotEnumChanged\0slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtLocalePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   39,    2, 0x0a /* Public */,
       7,    2,   44,    2, 0x08 /* Private */,
       8,    1,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QLocale,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QLocale,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtLocalePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtLocalePropertyManager *_t = static_cast<QtLocalePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QLocale(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QLocale(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtLocalePropertyManager::*_t)(QtProperty * , const QLocale & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtLocalePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtLocalePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtLocalePropertyManager.data,
      qt_meta_data_QtLocalePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtLocalePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtLocalePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtLocalePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtLocalePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtLocalePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtLocalePropertyManager::valueChanged(QtProperty * _t1, const QLocale & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtPointPropertyManager_t {
    QByteArrayData data[9];
    char stringdata0[108];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtPointPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtPointPropertyManager_t qt_meta_stringdata_QtPointPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtPointPropertyManager"
QT_MOC_LITERAL(1, 23, 12), // "valueChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 49, 8), // "property"
QT_MOC_LITERAL(5, 58, 3), // "val"
QT_MOC_LITERAL(6, 62, 8), // "setValue"
QT_MOC_LITERAL(7, 71, 14), // "slotIntChanged"
QT_MOC_LITERAL(8, 86, 21) // "slotPropertyDestroyed"

    },
    "QtPointPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue\0"
    "slotIntChanged\0slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtPointPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   39,    2, 0x0a /* Public */,
       7,    2,   44,    2, 0x08 /* Private */,
       8,    1,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QPoint,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QPoint,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtPointPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtPointPropertyManager *_t = static_cast<QtPointPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtPointPropertyManager::*_t)(QtProperty * , const QPoint & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtPointPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtPointPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtPointPropertyManager.data,
      qt_meta_data_QtPointPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtPointPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtPointPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtPointPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtPointPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtPointPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtPointPropertyManager::valueChanged(QtProperty * _t1, const QPoint & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtPointFPropertyManager_t {
    QByteArrayData data[12];
    char stringdata0[145];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtPointFPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtPointFPropertyManager_t qt_meta_stringdata_QtPointFPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtPointFPropertyManager"
QT_MOC_LITERAL(1, 24, 12), // "valueChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 8), // "property"
QT_MOC_LITERAL(5, 59, 3), // "val"
QT_MOC_LITERAL(6, 63, 15), // "decimalsChanged"
QT_MOC_LITERAL(7, 79, 4), // "prec"
QT_MOC_LITERAL(8, 84, 8), // "setValue"
QT_MOC_LITERAL(9, 93, 11), // "setDecimals"
QT_MOC_LITERAL(10, 105, 17), // "slotDoubleChanged"
QT_MOC_LITERAL(11, 123, 21) // "slotPropertyDestroyed"

    },
    "QtPointFPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0decimalsChanged\0"
    "prec\0setValue\0setDecimals\0slotDoubleChanged\0"
    "slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtPointFPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    2,   54,    2, 0x0a /* Public */,
       9,    2,   59,    2, 0x0a /* Public */,
      10,    2,   64,    2, 0x08 /* Private */,
      11,    1,   69,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QPointF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    7,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QPointF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtPointFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtPointFPropertyManager *_t = static_cast<QtPointFPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPointF(*)>(_a[2]))); break;
        case 1: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPointF(*)>(_a[2]))); break;
        case 3: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtPointFPropertyManager::*_t)(QtProperty * , const QPointF & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtPointFPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtPointFPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtPointFPropertyManager::decimalsChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtPointFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtPointFPropertyManager.data,
      qt_meta_data_QtPointFPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtPointFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtPointFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtPointFPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtPointFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtPointFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtPointFPropertyManager::valueChanged(QtProperty * _t1, const QPointF & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtPointFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtSizePropertyManager_t {
    QByteArrayData data[15];
    char stringdata0[165];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSizePropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSizePropertyManager_t qt_meta_stringdata_QtSizePropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtSizePropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 12), // "rangeChanged"
QT_MOC_LITERAL(7, 74, 6), // "minVal"
QT_MOC_LITERAL(8, 81, 6), // "maxVal"
QT_MOC_LITERAL(9, 88, 8), // "setValue"
QT_MOC_LITERAL(10, 97, 10), // "setMinimum"
QT_MOC_LITERAL(11, 108, 10), // "setMaximum"
QT_MOC_LITERAL(12, 119, 8), // "setRange"
QT_MOC_LITERAL(13, 128, 14), // "slotIntChanged"
QT_MOC_LITERAL(14, 143, 21) // "slotPropertyDestroyed"

    },
    "QtSizePropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0rangeChanged\0"
    "minVal\0maxVal\0setValue\0setMinimum\0"
    "setMaximum\0setRange\0slotIntChanged\0"
    "slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSizePropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   54,    2, 0x06 /* Public */,
       6,    3,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    2,   66,    2, 0x0a /* Public */,
      10,    2,   71,    2, 0x0a /* Public */,
      11,    2,   76,    2, 0x0a /* Public */,
      12,    3,   81,    2, 0x0a /* Public */,
      13,    2,   88,    2, 0x08 /* Private */,
      14,    1,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize, QMetaType::QSize,    4,    7,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize,    4,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSize, QMetaType::QSize,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtSizePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSizePropertyManager *_t = static_cast<QtSizePropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2])),(*reinterpret_cast< const QSize(*)>(_a[3]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 3: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 4: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 5: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2])),(*reinterpret_cast< const QSize(*)>(_a[3]))); break;
        case 6: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtSizePropertyManager::*_t)(QtProperty * , const QSize & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizePropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtSizePropertyManager::*_t)(QtProperty * , const QSize & , const QSize & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizePropertyManager::rangeChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtSizePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizePropertyManager.data,
      qt_meta_data_QtSizePropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtSizePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSizePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizePropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtSizePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtSizePropertyManager::valueChanged(QtProperty * _t1, const QSize & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtSizePropertyManager::rangeChanged(QtProperty * _t1, const QSize & _t2, const QSize & _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtSizeFPropertyManager_t {
    QByteArrayData data[18];
    char stringdata0[202];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSizeFPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSizeFPropertyManager_t qt_meta_stringdata_QtSizeFPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtSizeFPropertyManager"
QT_MOC_LITERAL(1, 23, 12), // "valueChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 49, 8), // "property"
QT_MOC_LITERAL(5, 58, 3), // "val"
QT_MOC_LITERAL(6, 62, 12), // "rangeChanged"
QT_MOC_LITERAL(7, 75, 6), // "minVal"
QT_MOC_LITERAL(8, 82, 6), // "maxVal"
QT_MOC_LITERAL(9, 89, 15), // "decimalsChanged"
QT_MOC_LITERAL(10, 105, 4), // "prec"
QT_MOC_LITERAL(11, 110, 8), // "setValue"
QT_MOC_LITERAL(12, 119, 10), // "setMinimum"
QT_MOC_LITERAL(13, 130, 10), // "setMaximum"
QT_MOC_LITERAL(14, 141, 8), // "setRange"
QT_MOC_LITERAL(15, 150, 11), // "setDecimals"
QT_MOC_LITERAL(16, 162, 17), // "slotDoubleChanged"
QT_MOC_LITERAL(17, 180, 21) // "slotPropertyDestroyed"

    },
    "QtSizeFPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0rangeChanged\0"
    "minVal\0maxVal\0decimalsChanged\0prec\0"
    "setValue\0setMinimum\0setMaximum\0setRange\0"
    "setDecimals\0slotDoubleChanged\0"
    "slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSizeFPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       6,    3,   69,    2, 0x06 /* Public */,
       9,    2,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,   81,    2, 0x0a /* Public */,
      12,    2,   86,    2, 0x0a /* Public */,
      13,    2,   91,    2, 0x0a /* Public */,
      14,    3,   96,    2, 0x0a /* Public */,
      15,    2,  103,    2, 0x0a /* Public */,
      16,    2,  108,    2, 0x08 /* Private */,
      17,    1,  113,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF, QMetaType::QSizeF,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   10,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF,    4,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizeF, QMetaType::QSizeF,    4,    7,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,   10,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtSizeFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSizeFPropertyManager *_t = static_cast<QtSizeFPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2])),(*reinterpret_cast< const QSizeF(*)>(_a[3]))); break;
        case 2: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 4: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 5: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 6: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2])),(*reinterpret_cast< const QSizeF(*)>(_a[3]))); break;
        case 7: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 9: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtSizeFPropertyManager::*_t)(QtProperty * , const QSizeF & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizeFPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtSizeFPropertyManager::*_t)(QtProperty * , const QSizeF & , const QSizeF & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizeFPropertyManager::rangeChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtSizeFPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizeFPropertyManager::decimalsChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject QtSizeFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizeFPropertyManager.data,
      qt_meta_data_QtSizeFPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtSizeFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSizeFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizeFPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtSizeFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizeFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QtSizeFPropertyManager::valueChanged(QtProperty * _t1, const QSizeF & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtSizeFPropertyManager::rangeChanged(QtProperty * _t1, const QSizeF & _t2, const QSizeF & _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtSizeFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_QtRectPropertyManager_t {
    QByteArrayData data[12];
    char stringdata0[150];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtRectPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtRectPropertyManager_t qt_meta_stringdata_QtRectPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtRectPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 17), // "constraintChanged"
QT_MOC_LITERAL(7, 79, 10), // "constraint"
QT_MOC_LITERAL(8, 90, 8), // "setValue"
QT_MOC_LITERAL(9, 99, 13), // "setConstraint"
QT_MOC_LITERAL(10, 113, 14), // "slotIntChanged"
QT_MOC_LITERAL(11, 128, 21) // "slotPropertyDestroyed"

    },
    "QtRectPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0constraintChanged\0"
    "constraint\0setValue\0setConstraint\0"
    "slotIntChanged\0slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtRectPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    2,   54,    2, 0x0a /* Public */,
       9,    2,   59,    2, 0x0a /* Public */,
      10,    2,   64,    2, 0x08 /* Private */,
      11,    1,   69,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRect,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRect,    4,    7,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRect,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRect,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtRectPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtRectPropertyManager *_t = static_cast<QtRectPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 1: _t->constraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 3: _t->setConstraint((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtRectPropertyManager::*_t)(QtProperty * , const QRect & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtRectPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtRectPropertyManager::*_t)(QtProperty * , const QRect & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtRectPropertyManager::constraintChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtRectPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtRectPropertyManager.data,
      qt_meta_data_QtRectPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtRectPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtRectPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtRectPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtRectPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtRectPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtRectPropertyManager::valueChanged(QtProperty * _t1, const QRect & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtRectPropertyManager::constraintChanged(QtProperty * _t1, const QRect & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtRectFPropertyManager_t {
    QByteArrayData data[15];
    char stringdata0[187];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtRectFPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtRectFPropertyManager_t qt_meta_stringdata_QtRectFPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtRectFPropertyManager"
QT_MOC_LITERAL(1, 23, 12), // "valueChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 49, 8), // "property"
QT_MOC_LITERAL(5, 58, 3), // "val"
QT_MOC_LITERAL(6, 62, 17), // "constraintChanged"
QT_MOC_LITERAL(7, 80, 10), // "constraint"
QT_MOC_LITERAL(8, 91, 15), // "decimalsChanged"
QT_MOC_LITERAL(9, 107, 4), // "prec"
QT_MOC_LITERAL(10, 112, 8), // "setValue"
QT_MOC_LITERAL(11, 121, 13), // "setConstraint"
QT_MOC_LITERAL(12, 135, 11), // "setDecimals"
QT_MOC_LITERAL(13, 147, 17), // "slotDoubleChanged"
QT_MOC_LITERAL(14, 165, 21) // "slotPropertyDestroyed"

    },
    "QtRectFPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0constraintChanged\0"
    "constraint\0decimalsChanged\0prec\0"
    "setValue\0setConstraint\0setDecimals\0"
    "slotDoubleChanged\0slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtRectFPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   54,    2, 0x06 /* Public */,
       6,    2,   59,    2, 0x06 /* Public */,
       8,    2,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    2,   69,    2, 0x0a /* Public */,
      11,    2,   74,    2, 0x0a /* Public */,
      12,    2,   79,    2, 0x0a /* Public */,
      13,    2,   84,    2, 0x08 /* Private */,
      14,    1,   89,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRectF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRectF,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    9,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRectF,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QRectF,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    9,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtRectFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtRectFPropertyManager *_t = static_cast<QtRectFPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 1: _t->constraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 2: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 4: _t->setConstraint((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 5: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 7: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtRectFPropertyManager::*_t)(QtProperty * , const QRectF & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtRectFPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtRectFPropertyManager::*_t)(QtProperty * , const QRectF & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtRectFPropertyManager::constraintChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtRectFPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtRectFPropertyManager::decimalsChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject QtRectFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtRectFPropertyManager.data,
      qt_meta_data_QtRectFPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtRectFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtRectFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtRectFPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtRectFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtRectFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtRectFPropertyManager::valueChanged(QtProperty * _t1, const QRectF & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtRectFPropertyManager::constraintChanged(QtProperty * _t1, const QRectF & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtRectFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_QtEnumPropertyManager_t {
    QByteArrayData data[14];
    char stringdata0[158];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtEnumPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtEnumPropertyManager_t qt_meta_stringdata_QtEnumPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtEnumPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 16), // "enumNamesChanged"
QT_MOC_LITERAL(7, 78, 5), // "names"
QT_MOC_LITERAL(8, 84, 16), // "enumIconsChanged"
QT_MOC_LITERAL(9, 101, 15), // "QMap<int,QIcon>"
QT_MOC_LITERAL(10, 117, 5), // "icons"
QT_MOC_LITERAL(11, 123, 8), // "setValue"
QT_MOC_LITERAL(12, 132, 12), // "setEnumNames"
QT_MOC_LITERAL(13, 145, 12) // "setEnumIcons"

    },
    "QtEnumPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0enumNamesChanged\0"
    "names\0enumIconsChanged\0QMap<int,QIcon>\0"
    "icons\0setValue\0setEnumNames\0setEnumIcons"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtEnumPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,
       8,    2,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,   59,    2, 0x0a /* Public */,
      12,    2,   64,    2, 0x0a /* Public */,
      13,    2,   69,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 9,    4,   10,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 9,    4,   10,

       0        // eod
};

void QtEnumPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtEnumPropertyManager *_t = static_cast<QtEnumPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->enumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->enumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setEnumNames((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 5: _t->setEnumIcons((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtEnumPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtEnumPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtEnumPropertyManager::*_t)(QtProperty * , const QStringList & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtEnumPropertyManager::enumNamesChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtEnumPropertyManager::*_t)(QtProperty * , const QMap<int,QIcon> & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtEnumPropertyManager::enumIconsChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject QtEnumPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtEnumPropertyManager.data,
      qt_meta_data_QtEnumPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtEnumPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtEnumPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtEnumPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtEnumPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtEnumPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtEnumPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtEnumPropertyManager::enumNamesChanged(QtProperty * _t1, const QStringList & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtEnumPropertyManager::enumIconsChanged(QtProperty * _t1, const QMap<int,QIcon> & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_QtFlagPropertyManager_t {
    QByteArrayData data[12];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtFlagPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtFlagPropertyManager_t qt_meta_stringdata_QtFlagPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtFlagPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 16), // "flagNamesChanged"
QT_MOC_LITERAL(7, 78, 5), // "names"
QT_MOC_LITERAL(8, 84, 8), // "setValue"
QT_MOC_LITERAL(9, 93, 12), // "setFlagNames"
QT_MOC_LITERAL(10, 106, 15), // "slotBoolChanged"
QT_MOC_LITERAL(11, 122, 21) // "slotPropertyDestroyed"

    },
    "QtFlagPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0flagNamesChanged\0"
    "names\0setValue\0setFlagNames\0slotBoolChanged\0"
    "slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFlagPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    2,   54,    2, 0x0a /* Public */,
       9,    2,   59,    2, 0x0a /* Public */,
      10,    2,   64,    2, 0x08 /* Private */,
      11,    1,   69,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    7,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    7,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtFlagPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtFlagPropertyManager *_t = static_cast<QtFlagPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->flagNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setFlagNames((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotBoolChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtFlagPropertyManager::*_t)(QtProperty * , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtFlagPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtFlagPropertyManager::*_t)(QtProperty * , const QStringList & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtFlagPropertyManager::flagNamesChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtFlagPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtFlagPropertyManager.data,
      qt_meta_data_QtFlagPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtFlagPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtFlagPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtFlagPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtFlagPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtFlagPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtFlagPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtFlagPropertyManager::flagNamesChanged(QtProperty * _t1, const QStringList & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_QtSizePolicyPropertyManager_t {
    QByteArrayData data[10];
    char stringdata0[129];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSizePolicyPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSizePolicyPropertyManager_t qt_meta_stringdata_QtSizePolicyPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 27), // "QtSizePolicyPropertyManager"
QT_MOC_LITERAL(1, 28, 12), // "valueChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 54, 8), // "property"
QT_MOC_LITERAL(5, 63, 3), // "val"
QT_MOC_LITERAL(6, 67, 8), // "setValue"
QT_MOC_LITERAL(7, 76, 14), // "slotIntChanged"
QT_MOC_LITERAL(8, 91, 15), // "slotEnumChanged"
QT_MOC_LITERAL(9, 107, 21) // "slotPropertyDestroyed"

    },
    "QtSizePolicyPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue\0"
    "slotIntChanged\0slotEnumChanged\0"
    "slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSizePolicyPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   44,    2, 0x0a /* Public */,
       7,    2,   49,    2, 0x08 /* Private */,
       8,    2,   54,    2, 0x08 /* Private */,
       9,    1,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizePolicy,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QSizePolicy,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtSizePolicyPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSizePolicyPropertyManager *_t = static_cast<QtSizePolicyPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizePolicy(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizePolicy(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtSizePolicyPropertyManager::*_t)(QtProperty * , const QSizePolicy & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtSizePolicyPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtSizePolicyPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizePolicyPropertyManager.data,
      qt_meta_data_QtSizePolicyPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtSizePolicyPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSizePolicyPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizePolicyPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtSizePolicyPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizePolicyPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtSizePolicyPropertyManager::valueChanged(QtProperty * _t1, const QSizePolicy & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtFontPropertyManager_t {
    QByteArrayData data[13];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtFontPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtFontPropertyManager_t qt_meta_stringdata_QtFontPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtFontPropertyManager"
QT_MOC_LITERAL(1, 22, 12), // "valueChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 48, 8), // "property"
QT_MOC_LITERAL(5, 57, 3), // "val"
QT_MOC_LITERAL(6, 61, 8), // "setValue"
QT_MOC_LITERAL(7, 70, 14), // "slotIntChanged"
QT_MOC_LITERAL(8, 85, 15), // "slotEnumChanged"
QT_MOC_LITERAL(9, 101, 15), // "slotBoolChanged"
QT_MOC_LITERAL(10, 117, 21), // "slotPropertyDestroyed"
QT_MOC_LITERAL(11, 139, 23), // "slotFontDatabaseChanged"
QT_MOC_LITERAL(12, 163, 29) // "slotFontDatabaseDelayedChange"

    },
    "QtFontPropertyManager\0valueChanged\0\0"
    "QtProperty*\0property\0val\0setValue\0"
    "slotIntChanged\0slotEnumChanged\0"
    "slotBoolChanged\0slotPropertyDestroyed\0"
    "slotFontDatabaseChanged\0"
    "slotFontDatabaseDelayedChange"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtFontPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   59,    2, 0x0a /* Public */,
       7,    2,   64,    2, 0x08 /* Private */,
       8,    2,   69,    2, 0x08 /* Private */,
       9,    2,   74,    2, 0x08 /* Private */,
      10,    1,   79,    2, 0x08 /* Private */,
      11,    0,   82,    2, 0x08 /* Private */,
      12,    0,   83,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QFont,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QFont,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QtFontPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtFontPropertyManager *_t = static_cast<QtFontPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotBoolChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 6: _t->d_func()->slotFontDatabaseChanged(); break;
        case 7: _t->d_func()->slotFontDatabaseDelayedChange(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtFontPropertyManager::*_t)(QtProperty * , const QFont & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtFontPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtFontPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtFontPropertyManager.data,
      qt_meta_data_QtFontPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtFontPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtFontPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtFontPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtFontPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtFontPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtFontPropertyManager::valueChanged(QtProperty * _t1, const QFont & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtColorPropertyManager_t {
    QByteArrayData data[9];
    char stringdata0[108];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtColorPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtColorPropertyManager_t qt_meta_stringdata_QtColorPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 22), // "QtColorPropertyManager"
QT_MOC_LITERAL(1, 23, 12), // "valueChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 49, 8), // "property"
QT_MOC_LITERAL(5, 58, 3), // "val"
QT_MOC_LITERAL(6, 62, 8), // "setValue"
QT_MOC_LITERAL(7, 71, 14), // "slotIntChanged"
QT_MOC_LITERAL(8, 86, 21) // "slotPropertyDestroyed"

    },
    "QtColorPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue\0"
    "slotIntChanged\0slotPropertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtColorPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   39,    2, 0x0a /* Public */,
       7,    2,   44,    2, 0x08 /* Private */,
       8,    1,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QColor,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QColor,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,
    QMetaType::Void, 0x80000000 | 3,    2,

       0        // eod
};

void QtColorPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtColorPropertyManager *_t = static_cast<QtColorPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtColorPropertyManager::*_t)(QtProperty * , const QColor & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtColorPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtColorPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtColorPropertyManager.data,
      qt_meta_data_QtColorPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtColorPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtColorPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtColorPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtColorPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtColorPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtColorPropertyManager::valueChanged(QtProperty * _t1, const QColor & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtCursorPropertyManager_t {
    QByteArrayData data[7];
    char stringdata0[72];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtCursorPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtCursorPropertyManager_t qt_meta_stringdata_QtCursorPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtCursorPropertyManager"
QT_MOC_LITERAL(1, 24, 12), // "valueChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 50, 8), // "property"
QT_MOC_LITERAL(5, 59, 3), // "val"
QT_MOC_LITERAL(6, 63, 8) // "setValue"

    },
    "QtCursorPropertyManager\0valueChanged\0"
    "\0QtProperty*\0property\0val\0setValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtCursorPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   29,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QCursor,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QCursor,    4,    5,

       0        // eod
};

void QtCursorPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtCursorPropertyManager *_t = static_cast<QtCursorPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtCursorPropertyManager::*_t)(QtProperty * , const QCursor & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtCursorPropertyManager::valueChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtCursorPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtCursorPropertyManager.data,
      qt_meta_data_QtCursorPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtCursorPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtCursorPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtCursorPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtCursorPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtCursorPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtCursorPropertyManager::valueChanged(QtProperty * _t1, const QCursor & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
