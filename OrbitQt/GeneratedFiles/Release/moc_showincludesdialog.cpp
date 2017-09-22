/****************************************************************************
** Meta object code from reading C++ file 'showincludesdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../showincludesdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'showincludesdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ShowIncludesDialog_t {
    QByteArrayData data[11];
    char stringdata0[190];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ShowIncludesDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ShowIncludesDialog_t qt_meta_stringdata_ShowIncludesDialog = {
    {
QT_MOC_LITERAL(0, 0, 18), // "ShowIncludesDialog"
QT_MOC_LITERAL(1, 19, 19), // "onCustomContextMenu"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 5), // "point"
QT_MOC_LITERAL(4, 46, 13), // "OnMenuClicked"
QT_MOC_LITERAL(5, 60, 28), // "on_plainTextEdit_textChanged"
QT_MOC_LITERAL(6, 89, 23), // "on_lineEdit_textChanged"
QT_MOC_LITERAL(7, 113, 4), // "arg1"
QT_MOC_LITERAL(8, 118, 21), // "on_pushButton_clicked"
QT_MOC_LITERAL(9, 140, 23), // "on_pushButton_2_clicked"
QT_MOC_LITERAL(10, 164, 25) // "on_lineEdit_2_textChanged"

    },
    "ShowIncludesDialog\0onCustomContextMenu\0"
    "\0point\0OnMenuClicked\0on_plainTextEdit_textChanged\0"
    "on_lineEdit_textChanged\0arg1\0"
    "on_pushButton_clicked\0on_pushButton_2_clicked\0"
    "on_lineEdit_2_textChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ShowIncludesDialog[] = {

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
       1,    1,   49,    2, 0x0a /* Public */,
       4,    1,   52,    2, 0x0a /* Public */,
       5,    0,   55,    2, 0x08 /* Private */,
       6,    1,   56,    2, 0x08 /* Private */,
       8,    0,   59,    2, 0x08 /* Private */,
       9,    0,   60,    2, 0x08 /* Private */,
      10,    1,   61,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QPoint,    3,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,

       0        // eod
};

void ShowIncludesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ShowIncludesDialog *_t = static_cast<ShowIncludesDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCustomContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 1: _t->OnMenuClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_plainTextEdit_textChanged(); break;
        case 3: _t->on_lineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->on_pushButton_clicked(); break;
        case 5: _t->on_pushButton_2_clicked(); break;
        case 6: _t->on_lineEdit_2_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ShowIncludesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ShowIncludesDialog.data,
      qt_meta_data_ShowIncludesDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ShowIncludesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ShowIncludesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ShowIncludesDialog.stringdata0))
        return static_cast<void*>(const_cast< ShowIncludesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ShowIncludesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
