/****************************************************************************
** Meta object code from reading C++ file 'orbittreeview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../orbittreeview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'orbittreeview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OrbitTreeView_t {
    QByteArrayData data[20];
    char stringdata0[184];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OrbitTreeView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OrbitTreeView_t qt_meta_stringdata_OrbitTreeView = {
    {
QT_MOC_LITERAL(0, 0, 13), // "OrbitTreeView"
QT_MOC_LITERAL(1, 14, 13), // "columnResized"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 6), // "column"
QT_MOC_LITERAL(4, 36, 7), // "oldSize"
QT_MOC_LITERAL(5, 44, 7), // "newSize"
QT_MOC_LITERAL(6, 52, 6), // "OnSort"
QT_MOC_LITERAL(7, 59, 9), // "a_Section"
QT_MOC_LITERAL(8, 69, 13), // "Qt::SortOrder"
QT_MOC_LITERAL(9, 83, 7), // "a_Order"
QT_MOC_LITERAL(10, 91, 7), // "OnTimer"
QT_MOC_LITERAL(11, 99, 9), // "OnClicked"
QT_MOC_LITERAL(12, 109, 5), // "index"
QT_MOC_LITERAL(13, 115, 15), // "ShowContextMenu"
QT_MOC_LITERAL(14, 131, 3), // "pos"
QT_MOC_LITERAL(15, 135, 13), // "OnMenuClicked"
QT_MOC_LITERAL(16, 149, 7), // "a_Index"
QT_MOC_LITERAL(17, 157, 14), // "OnRangeChanged"
QT_MOC_LITERAL(18, 172, 5), // "a_Min"
QT_MOC_LITERAL(19, 178, 5) // "a_Max"

    },
    "OrbitTreeView\0columnResized\0\0column\0"
    "oldSize\0newSize\0OnSort\0a_Section\0"
    "Qt::SortOrder\0a_Order\0OnTimer\0OnClicked\0"
    "index\0ShowContextMenu\0pos\0OnMenuClicked\0"
    "a_Index\0OnRangeChanged\0a_Min\0a_Max"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OrbitTreeView[] = {

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
       1,    3,   49,    2, 0x0a /* Public */,
       6,    2,   56,    2, 0x08 /* Private */,
      10,    0,   61,    2, 0x08 /* Private */,
      11,    1,   62,    2, 0x08 /* Private */,
      13,    1,   65,    2, 0x08 /* Private */,
      15,    1,   68,    2, 0x08 /* Private */,
      17,    2,   71,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    7,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex,   12,
    QMetaType::Void, QMetaType::QPoint,   14,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   18,   19,

       0        // eod
};

void OrbitTreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OrbitTreeView *_t = static_cast<OrbitTreeView *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->columnResized((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->OnSort((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< Qt::SortOrder(*)>(_a[2]))); break;
        case 2: _t->OnTimer(); break;
        case 3: _t->OnClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->ShowContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 5: _t->OnMenuClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->OnRangeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject OrbitTreeView::staticMetaObject = {
    { &QTreeView::staticMetaObject, qt_meta_stringdata_OrbitTreeView.data,
      qt_meta_data_OrbitTreeView,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *OrbitTreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrbitTreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_OrbitTreeView.stringdata0))
        return static_cast<void*>(const_cast< OrbitTreeView*>(this));
    return QTreeView::qt_metacast(_clname);
}

int OrbitTreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
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
