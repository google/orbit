/****************************************************************************
** Meta object code from reading C++ file 'orbitcodeeditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../orbitcodeeditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'orbitcodeeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OrbitCodeEditor_t {
    QByteArrayData data[8];
    char stringdata0[131];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OrbitCodeEditor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OrbitCodeEditor_t qt_meta_stringdata_OrbitCodeEditor = {
    {
QT_MOC_LITERAL(0, 0, 15), // "OrbitCodeEditor"
QT_MOC_LITERAL(1, 16, 25), // "updateLineNumberAreaWidth"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 13), // "newBlockCount"
QT_MOC_LITERAL(4, 57, 20), // "highlightCurrentLine"
QT_MOC_LITERAL(5, 78, 20), // "updateLineNumberArea"
QT_MOC_LITERAL(6, 99, 17), // "OnFindTextEntered"
QT_MOC_LITERAL(7, 117, 13) // "OnSaveMapFile"

    },
    "OrbitCodeEditor\0updateLineNumberAreaWidth\0"
    "\0newBlockCount\0highlightCurrentLine\0"
    "updateLineNumberArea\0OnFindTextEntered\0"
    "OnSaveMapFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OrbitCodeEditor[] = {

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
       1,    1,   39,    2, 0x08 /* Private */,
       4,    0,   42,    2, 0x08 /* Private */,
       5,    2,   43,    2, 0x08 /* Private */,
       6,    1,   48,    2, 0x08 /* Private */,
       7,    0,   51,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void OrbitCodeEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OrbitCodeEditor *_t = static_cast<OrbitCodeEditor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateLineNumberAreaWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->highlightCurrentLine(); break;
        case 2: _t->updateLineNumberArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->OnFindTextEntered((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->OnSaveMapFile(); break;
        default: ;
        }
    }
}

const QMetaObject OrbitCodeEditor::staticMetaObject = {
    { &QPlainTextEdit::staticMetaObject, qt_meta_stringdata_OrbitCodeEditor.data,
      qt_meta_data_OrbitCodeEditor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *OrbitCodeEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrbitCodeEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_OrbitCodeEditor.stringdata0))
        return static_cast<void*>(const_cast< OrbitCodeEditor*>(this));
    return QPlainTextEdit::qt_metacast(_clname);
}

int OrbitCodeEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_Highlighter_t {
    QByteArrayData data[1];
    char stringdata0[12];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Highlighter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Highlighter_t qt_meta_stringdata_Highlighter = {
    {
QT_MOC_LITERAL(0, 0, 11) // "Highlighter"

    },
    "Highlighter"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Highlighter[] = {

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

void Highlighter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject Highlighter::staticMetaObject = {
    { &QSyntaxHighlighter::staticMetaObject, qt_meta_stringdata_Highlighter.data,
      qt_meta_data_Highlighter,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Highlighter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Highlighter::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Highlighter.stringdata0))
        return static_cast<void*>(const_cast< Highlighter*>(this));
    return QSyntaxHighlighter::qt_metacast(_clname);
}

int Highlighter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSyntaxHighlighter::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
