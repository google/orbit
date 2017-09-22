/****************************************************************************
** Meta object code from reading C++ file 'qtbuttonpropertybrowser.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qtbuttonpropertybrowser.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtbuttonpropertybrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtButtonPropertyBrowser_t {
    QByteArrayData data[9];
    char stringdata0[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtButtonPropertyBrowser_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtButtonPropertyBrowser_t qt_meta_stringdata_QtButtonPropertyBrowser = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtButtonPropertyBrowser"
QT_MOC_LITERAL(1, 24, 9), // "collapsed"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 14), // "QtBrowserItem*"
QT_MOC_LITERAL(4, 50, 4), // "item"
QT_MOC_LITERAL(5, 55, 8), // "expanded"
QT_MOC_LITERAL(6, 64, 10), // "slotUpdate"
QT_MOC_LITERAL(7, 75, 19), // "slotEditorDestroyed"
QT_MOC_LITERAL(8, 95, 11) // "slotToggled"

    },
    "QtButtonPropertyBrowser\0collapsed\0\0"
    "QtBrowserItem*\0item\0expanded\0slotUpdate\0"
    "slotEditorDestroyed\0slotToggled"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtButtonPropertyBrowser[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   45,    2, 0x08 /* Private */,
       7,    0,   46,    2, 0x08 /* Private */,
       8,    1,   47,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void QtButtonPropertyBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtButtonPropertyBrowser *_t = static_cast<QtButtonPropertyBrowser *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->collapsed((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 1: _t->expanded((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotUpdate(); break;
        case 3: _t->d_func()->slotEditorDestroyed(); break;
        case 4: _t->d_func()->slotToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtButtonPropertyBrowser::*_t)(QtBrowserItem * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtButtonPropertyBrowser::collapsed)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtButtonPropertyBrowser::*_t)(QtBrowserItem * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtButtonPropertyBrowser::expanded)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject QtButtonPropertyBrowser::staticMetaObject = {
    { &QtAbstractPropertyBrowser::staticMetaObject, qt_meta_stringdata_QtButtonPropertyBrowser.data,
      qt_meta_data_QtButtonPropertyBrowser,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtButtonPropertyBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtButtonPropertyBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtButtonPropertyBrowser.stringdata0))
        return static_cast<void*>(const_cast< QtButtonPropertyBrowser*>(this));
    return QtAbstractPropertyBrowser::qt_metacast(_clname);
}

int QtButtonPropertyBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyBrowser::qt_metacall(_c, _id, _a);
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
void QtButtonPropertyBrowser::collapsed(QtBrowserItem * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtButtonPropertyBrowser::expanded(QtBrowserItem * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
