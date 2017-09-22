/****************************************************************************
** Meta object code from reading C++ file 'qtpropertybrowser.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qtpropertybrowser.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtpropertybrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtAbstractPropertyManager_t {
    QByteArrayData data[10];
    char stringdata0[128];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAbstractPropertyManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAbstractPropertyManager_t qt_meta_stringdata_QtAbstractPropertyManager = {
    {
QT_MOC_LITERAL(0, 0, 25), // "QtAbstractPropertyManager"
QT_MOC_LITERAL(1, 26, 16), // "propertyInserted"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 11), // "QtProperty*"
QT_MOC_LITERAL(4, 56, 8), // "property"
QT_MOC_LITERAL(5, 65, 6), // "parent"
QT_MOC_LITERAL(6, 72, 5), // "after"
QT_MOC_LITERAL(7, 78, 15), // "propertyChanged"
QT_MOC_LITERAL(8, 94, 15), // "propertyRemoved"
QT_MOC_LITERAL(9, 110, 17) // "propertyDestroyed"

    },
    "QtAbstractPropertyManager\0propertyInserted\0"
    "\0QtProperty*\0property\0parent\0after\0"
    "propertyChanged\0propertyRemoved\0"
    "propertyDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAbstractPropertyManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x06 /* Public */,
       7,    1,   41,    2, 0x06 /* Public */,
       8,    2,   44,    2, 0x06 /* Public */,
       9,    1,   49,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, 0x80000000 | 3,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void QtAbstractPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtAbstractPropertyManager *_t = static_cast<QtAbstractPropertyManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->propertyInserted((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< QtProperty*(*)>(_a[3]))); break;
        case 1: _t->propertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 2: _t->propertyRemoved((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2]))); break;
        case 3: _t->propertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtAbstractPropertyManager::*_t)(QtProperty * , QtProperty * , QtProperty * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtAbstractPropertyManager::propertyInserted)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtAbstractPropertyManager::*_t)(QtProperty * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtAbstractPropertyManager::propertyChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (QtAbstractPropertyManager::*_t)(QtProperty * , QtProperty * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtAbstractPropertyManager::propertyRemoved)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (QtAbstractPropertyManager::*_t)(QtProperty * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtAbstractPropertyManager::propertyDestroyed)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject QtAbstractPropertyManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtAbstractPropertyManager.data,
      qt_meta_data_QtAbstractPropertyManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtAbstractPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtAbstractPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractPropertyManager.stringdata0))
        return static_cast<void*>(const_cast< QtAbstractPropertyManager*>(this));
    return QObject::qt_metacast(_clname);
}

int QtAbstractPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void QtAbstractPropertyManager::propertyInserted(QtProperty * _t1, QtProperty * _t2, QtProperty * _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtAbstractPropertyManager::propertyChanged(QtProperty * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtAbstractPropertyManager::propertyRemoved(QtProperty * _t1, QtProperty * _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtAbstractPropertyManager::propertyDestroyed(QtProperty * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
struct qt_meta_stringdata_QtAbstractEditorFactoryBase_t {
    QByteArrayData data[4];
    char stringdata0[54];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAbstractEditorFactoryBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAbstractEditorFactoryBase_t qt_meta_stringdata_QtAbstractEditorFactoryBase = {
    {
QT_MOC_LITERAL(0, 0, 27), // "QtAbstractEditorFactoryBase"
QT_MOC_LITERAL(1, 28, 16), // "managerDestroyed"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 7) // "manager"

    },
    "QtAbstractEditorFactoryBase\0"
    "managerDestroyed\0\0manager"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAbstractEditorFactoryBase[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::QObjectStar,    3,

       0        // eod
};

void QtAbstractEditorFactoryBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtAbstractEditorFactoryBase *_t = static_cast<QtAbstractEditorFactoryBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->managerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtAbstractEditorFactoryBase::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtAbstractEditorFactoryBase.data,
      qt_meta_data_QtAbstractEditorFactoryBase,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtAbstractEditorFactoryBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtAbstractEditorFactoryBase::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractEditorFactoryBase.stringdata0))
        return static_cast<void*>(const_cast< QtAbstractEditorFactoryBase*>(this));
    return QObject::qt_metacast(_clname);
}

int QtAbstractEditorFactoryBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_QtAbstractPropertyBrowser_t {
    QByteArrayData data[14];
    char stringdata0[225];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAbstractPropertyBrowser_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAbstractPropertyBrowser_t qt_meta_stringdata_QtAbstractPropertyBrowser = {
    {
QT_MOC_LITERAL(0, 0, 25), // "QtAbstractPropertyBrowser"
QT_MOC_LITERAL(1, 26, 18), // "currentItemChanged"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 14), // "QtBrowserItem*"
QT_MOC_LITERAL(4, 61, 11), // "addProperty"
QT_MOC_LITERAL(5, 73, 11), // "QtProperty*"
QT_MOC_LITERAL(6, 85, 8), // "property"
QT_MOC_LITERAL(7, 94, 14), // "insertProperty"
QT_MOC_LITERAL(8, 109, 13), // "afterProperty"
QT_MOC_LITERAL(9, 123, 14), // "removeProperty"
QT_MOC_LITERAL(10, 138, 20), // "slotPropertyInserted"
QT_MOC_LITERAL(11, 159, 19), // "slotPropertyRemoved"
QT_MOC_LITERAL(12, 179, 21), // "slotPropertyDestroyed"
QT_MOC_LITERAL(13, 201, 23) // "slotPropertyDataChanged"

    },
    "QtAbstractPropertyBrowser\0currentItemChanged\0"
    "\0QtBrowserItem*\0addProperty\0QtProperty*\0"
    "property\0insertProperty\0afterProperty\0"
    "removeProperty\0slotPropertyInserted\0"
    "slotPropertyRemoved\0slotPropertyDestroyed\0"
    "slotPropertyDataChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAbstractPropertyBrowser[] = {

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
       1,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   57,    2, 0x0a /* Public */,
       7,    2,   60,    2, 0x0a /* Public */,
       9,    1,   65,    2, 0x0a /* Public */,
      10,    3,   68,    2, 0x08 /* Private */,
      11,    2,   75,    2, 0x08 /* Private */,
      12,    1,   80,    2, 0x08 /* Private */,
      13,    1,   83,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    0x80000000 | 3, 0x80000000 | 5,    6,
    0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 5,    6,    8,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5,    2,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5,    2,

       0        // eod
};

void QtAbstractPropertyBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtAbstractPropertyBrowser *_t = static_cast<QtAbstractPropertyBrowser *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentItemChanged((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 1: { QtBrowserItem* _r = _t->addProperty((*reinterpret_cast< QtProperty*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QtBrowserItem**>(_a[0]) = _r; }  break;
        case 2: { QtBrowserItem* _r = _t->insertProperty((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QtBrowserItem**>(_a[0]) = _r; }  break;
        case 3: _t->removeProperty((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotPropertyInserted((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< QtProperty*(*)>(_a[3]))); break;
        case 5: _t->d_func()->slotPropertyRemoved((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2]))); break;
        case 6: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 7: _t->d_func()->slotPropertyDataChanged((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtAbstractPropertyBrowser::*_t)(QtBrowserItem * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtAbstractPropertyBrowser::currentItemChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtAbstractPropertyBrowser::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtAbstractPropertyBrowser.data,
      qt_meta_data_QtAbstractPropertyBrowser,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtAbstractPropertyBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtAbstractPropertyBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractPropertyBrowser.stringdata0))
        return static_cast<void*>(const_cast< QtAbstractPropertyBrowser*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtAbstractPropertyBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void QtAbstractPropertyBrowser::currentItemChanged(QtBrowserItem * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
