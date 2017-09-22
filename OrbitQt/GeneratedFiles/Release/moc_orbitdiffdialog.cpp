/****************************************************************************
** Meta object code from reading C++ file 'orbitdiffdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../orbitdiffdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'orbitdiffdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OrbitDiffDialog_t {
    QByteArrayData data[8];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OrbitDiffDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OrbitDiffDialog_t qt_meta_stringdata_OrbitDiffDialog = {
    {
QT_MOC_LITERAL(0, 0, 15), // "OrbitDiffDialog"
QT_MOC_LITERAL(1, 16, 21), // "on_pushButton_clicked"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 23), // "on_BrowseButton_clicked"
QT_MOC_LITERAL(4, 63, 30), // "on_DiffExeLineEdit_textChanged"
QT_MOC_LITERAL(5, 94, 4), // "arg1"
QT_MOC_LITERAL(6, 99, 27), // "on_ArgsLineEdit_textChanged"
QT_MOC_LITERAL(7, 127, 21) // "on_buttonBox_accepted"

    },
    "OrbitDiffDialog\0on_pushButton_clicked\0"
    "\0on_BrowseButton_clicked\0"
    "on_DiffExeLineEdit_textChanged\0arg1\0"
    "on_ArgsLineEdit_textChanged\0"
    "on_buttonBox_accepted"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OrbitDiffDialog[] = {

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
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    1,   41,    2, 0x08 /* Private */,
       6,    1,   44,    2, 0x08 /* Private */,
       7,    0,   47,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,

       0        // eod
};

void OrbitDiffDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OrbitDiffDialog *_t = static_cast<OrbitDiffDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_pushButton_clicked(); break;
        case 1: _t->on_BrowseButton_clicked(); break;
        case 2: _t->on_DiffExeLineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_ArgsLineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->on_buttonBox_accepted(); break;
        default: ;
        }
    }
}

const QMetaObject OrbitDiffDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_OrbitDiffDialog.data,
      qt_meta_data_OrbitDiffDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *OrbitDiffDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrbitDiffDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_OrbitDiffDialog.stringdata0))
        return static_cast<void*>(const_cast< OrbitDiffDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int OrbitDiffDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
