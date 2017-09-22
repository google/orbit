/****************************************************************************
** Meta object code from reading C++ file 'qtpropertybrowserutils_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtpropertybrowser/qtpropertybrowserutils_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtpropertybrowserutils_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtBoolEdit_t {
    QByteArrayData data[3];
    char stringdata0[20];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtBoolEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtBoolEdit_t qt_meta_stringdata_QtBoolEdit = {
    {
QT_MOC_LITERAL(0, 0, 10), // "QtBoolEdit"
QT_MOC_LITERAL(1, 11, 7), // "toggled"
QT_MOC_LITERAL(2, 19, 0) // ""

    },
    "QtBoolEdit\0toggled\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtBoolEdit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void QtBoolEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtBoolEdit *_t = static_cast<QtBoolEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtBoolEdit::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtBoolEdit::toggled)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtBoolEdit::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtBoolEdit.data,
      qt_meta_data_QtBoolEdit,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtBoolEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtBoolEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtBoolEdit.stringdata0))
        return static_cast<void*>(const_cast< QtBoolEdit*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtBoolEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QtBoolEdit::toggled(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_QtKeySequenceEdit_t {
    QByteArrayData data[6];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtKeySequenceEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtKeySequenceEdit_t qt_meta_stringdata_QtKeySequenceEdit = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtKeySequenceEdit"
QT_MOC_LITERAL(1, 18, 18), // "keySequenceChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 8), // "sequence"
QT_MOC_LITERAL(4, 47, 14), // "setKeySequence"
QT_MOC_LITERAL(5, 62, 17) // "slotClearShortcut"

    },
    "QtKeySequenceEdit\0keySequenceChanged\0"
    "\0sequence\0setKeySequence\0slotClearShortcut"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtKeySequenceEdit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   32,    2, 0x0a /* Public */,
       5,    0,   35,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QKeySequence,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QKeySequence,    3,
    QMetaType::Void,

       0        // eod
};

void QtKeySequenceEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtKeySequenceEdit *_t = static_cast<QtKeySequenceEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->keySequenceChanged((*reinterpret_cast< const QKeySequence(*)>(_a[1]))); break;
        case 1: _t->setKeySequence((*reinterpret_cast< const QKeySequence(*)>(_a[1]))); break;
        case 2: _t->slotClearShortcut(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QtKeySequenceEdit::*_t)(const QKeySequence & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QtKeySequenceEdit::keySequenceChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject QtKeySequenceEdit::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtKeySequenceEdit.data,
      qt_meta_data_QtKeySequenceEdit,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtKeySequenceEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtKeySequenceEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtKeySequenceEdit.stringdata0))
        return static_cast<void*>(const_cast< QtKeySequenceEdit*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtKeySequenceEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void QtKeySequenceEdit::keySequenceChanged(const QKeySequence & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
