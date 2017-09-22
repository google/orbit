/****************************************************************************
** Meta object code from reading C++ file 'qttreepropertybrowser.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qttreepropertybrowser.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qttreepropertybrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtTreePropertyBrowser_t {
    QByteArrayData data[23];
    char stringdata0[337];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtTreePropertyBrowser_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtTreePropertyBrowser_t qt_meta_stringdata_QtTreePropertyBrowser = {
    {
QT_MOC_LITERAL(0, 0, 21), // "QtTreePropertyBrowser"
QT_MOC_LITERAL(1, 22, 9), // "collapsed"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 14), // "QtBrowserItem*"
QT_MOC_LITERAL(4, 48, 4), // "item"
QT_MOC_LITERAL(5, 53, 8), // "expanded"
QT_MOC_LITERAL(6, 62, 13), // "slotCollapsed"
QT_MOC_LITERAL(7, 76, 12), // "slotExpanded"
QT_MOC_LITERAL(8, 89, 29), // "slotCurrentBrowserItemChanged"
QT_MOC_LITERAL(9, 119, 26), // "slotCurrentTreeItemChanged"
QT_MOC_LITERAL(10, 146, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(11, 163, 11), // "indentation"
QT_MOC_LITERAL(12, 175, 15), // "rootIsDecorated"
QT_MOC_LITERAL(13, 191, 20), // "alternatingRowColors"
QT_MOC_LITERAL(14, 212, 13), // "headerVisible"
QT_MOC_LITERAL(15, 226, 10), // "resizeMode"
QT_MOC_LITERAL(16, 237, 10), // "ResizeMode"
QT_MOC_LITERAL(17, 248, 16), // "splitterPosition"
QT_MOC_LITERAL(18, 265, 28), // "propertiesWithoutValueMarked"
QT_MOC_LITERAL(19, 294, 11), // "Interactive"
QT_MOC_LITERAL(20, 306, 7), // "Stretch"
QT_MOC_LITERAL(21, 314, 5), // "Fixed"
QT_MOC_LITERAL(22, 320, 16) // "ResizeToContents"

    },
    "QtTreePropertyBrowser\0collapsed\0\0"
    "QtBrowserItem*\0item\0expanded\0slotCollapsed\0"
    "slotExpanded\0slotCurrentBrowserItemChanged\0"
    "slotCurrentTreeItemChanged\0QTreeWidgetItem*\0"
    "indentation\0rootIsDecorated\0"
    "alternatingRowColors\0headerVisible\0"
    "resizeMode\0ResizeMode\0splitterPosition\0"
    "propertiesWithoutValueMarked\0Interactive\0"
    "Stretch\0Fixed\0ResizeToContents"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtTreePropertyBrowser[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       7,   64, // properties
       1,   85, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    1,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   50,    2, 0x08 /* Private */,
       7,    1,   53,    2, 0x08 /* Private */,
       8,    1,   56,    2, 0x08 /* Private */,
       9,    2,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::QModelIndex,    2,
    QMetaType::Void, QMetaType::QModelIndex,    2,
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 10,    2,    2,

 // properties: name, type, flags
      11, QMetaType::Int, 0x00095103,
      12, QMetaType::Bool, 0x00095103,
      13, QMetaType::Bool, 0x00095103,
      14, QMetaType::Bool, 0x00095103,
      15, 0x80000000 | 16, 0x0009510b,
      17, QMetaType::Int, 0x00095103,
      18, QMetaType::Bool, 0x00095103,

 // enums: name, flags, count, data
      16, 0x0,    4,   89,

 // enum data: key, value
      19, uint(QtTreePropertyBrowser::Interactive),
      20, uint(QtTreePropertyBrowser::Stretch),
      21, uint(QtTreePropertyBrowser::Fixed),
      22, uint(QtTreePropertyBrowser::ResizeToContents),

       0        // eod
};

void QtTreePropertyBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtTreePropertyBrowser *_t = static_cast<QtTreePropertyBrowser *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->collapsed((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 1: _t->expanded((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotCollapsed((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 3: _t->d_func()->slotExpanded((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotCurrentBrowserItemChanged((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 5: _t->d_func()->slotCurrentTreeItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtTreePropertyBrowser::*_t)(QtBrowserItem * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtTreePropertyBrowser::collapsed)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (QtTreePropertyBrowser::*_t)(QtBrowserItem * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtTreePropertyBrowser::expanded)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        QtTreePropertyBrowser *_t = static_cast<QtTreePropertyBrowser *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->indentation(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->rootIsDecorated(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->alternatingRowColors(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->isHeaderVisible(); break;
        case 4: *reinterpret_cast< ResizeMode*>(_v) = _t->resizeMode(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->splitterPosition(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->propertiesWithoutValueMarked(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        QtTreePropertyBrowser *_t = static_cast<QtTreePropertyBrowser *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setIndentation(*reinterpret_cast< int*>(_v)); break;
        case 1: _t->setRootIsDecorated(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->setAlternatingRowColors(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setHeaderVisible(*reinterpret_cast< bool*>(_v)); break;
        case 4: _t->setResizeMode(*reinterpret_cast< ResizeMode*>(_v)); break;
        case 5: _t->setSplitterPosition(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setPropertiesWithoutValueMarked(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

const QMetaObject QtTreePropertyBrowser::staticMetaObject = {
    { &QtAbstractPropertyBrowser::staticMetaObject, qt_meta_stringdata_QtTreePropertyBrowser.data,
      qt_meta_data_QtTreePropertyBrowser,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtTreePropertyBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtTreePropertyBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtTreePropertyBrowser.stringdata0))
        return static_cast<void*>(const_cast< QtTreePropertyBrowser*>(this));
    return QtAbstractPropertyBrowser::qt_metacast(_clname);
}

int QtTreePropertyBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyBrowser::qt_metacall(_c, _id, _a);
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
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QtTreePropertyBrowser::collapsed(QtBrowserItem * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtTreePropertyBrowser::expanded(QtBrowserItem * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
