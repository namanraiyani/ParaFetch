/****************************************************************************
** Meta object code from reading C++ file 'downloadworker.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../downloadworker.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'downloadworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_DownloadWorker_t {
    uint offsetsAndSizes[62];
    char stringdata0[15];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[3];
    char stringdata4[16];
    char stringdata5[14];
    char stringdata6[16];
    char stringdata7[10];
    char stringdata8[6];
    char stringdata9[4];
    char stringdata10[21];
    char stringdata11[27];
    char stringdata12[7];
    char stringdata13[17];
    char stringdata14[8];
    char stringdata15[8];
    char stringdata16[15];
    char stringdata17[11];
    char stringdata18[14];
    char stringdata19[7];
    char stringdata20[14];
    char stringdata21[4];
    char stringdata22[11];
    char stringdata23[14];
    char stringdata24[15];
    char stringdata25[15];
    char stringdata26[14];
    char stringdata27[6];
    char stringdata28[12];
    char stringdata29[15];
    char stringdata30[23];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_DownloadWorker_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_DownloadWorker_t qt_meta_stringdata_DownloadWorker = {
    {
        QT_MOC_LITERAL(0, 14),  // "DownloadWorker"
        QT_MOC_LITERAL(15, 19),  // "downloadIDGenerated"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 2),  // "id"
        QT_MOC_LITERAL(39, 15),  // "progressUpdated"
        QT_MOC_LITERAL(55, 13),  // "totalProgress"
        QT_MOC_LITERAL(69, 15),  // "totalDownloaded"
        QT_MOC_LITERAL(85, 9),  // "totalSize"
        QT_MOC_LITERAL(95, 5),  // "speed"
        QT_MOC_LITERAL(101, 3),  // "eta"
        QT_MOC_LITERAL(105, 20),  // "chunkProgressUpdated"
        QT_MOC_LITERAL(126, 26),  // "std::vector<ChunkProgress>"
        QT_MOC_LITERAL(153, 6),  // "chunks"
        QT_MOC_LITERAL(160, 16),  // "downloadFinished"
        QT_MOC_LITERAL(177, 7),  // "success"
        QT_MOC_LITERAL(185, 7),  // "message"
        QT_MOC_LITERAL(193, 14),  // "downloadPaused"
        QT_MOC_LITERAL(208, 10),  // "downloadId"
        QT_MOC_LITERAL(219, 13),  // "statusChanged"
        QT_MOC_LITERAL(233, 6),  // "status"
        QT_MOC_LITERAL(240, 13),  // "startDownload"
        QT_MOC_LITERAL(254, 3),  // "url"
        QT_MOC_LITERAL(258, 10),  // "outputPath"
        QT_MOC_LITERAL(269, 13),  // "pauseDownload"
        QT_MOC_LITERAL(283, 14),  // "resumeDownload"
        QT_MOC_LITERAL(298, 14),  // "cancelDownload"
        QT_MOC_LITERAL(313, 13),  // "setSpeedLimit"
        QT_MOC_LITERAL(327, 5),  // "limit"
        QT_MOC_LITERAL(333, 11),  // "performWork"
        QT_MOC_LITERAL(345, 14),  // "updateProgress"
        QT_MOC_LITERAL(360, 22)   // "attemptNetworkRecovery"
    },
    "DownloadWorker",
    "downloadIDGenerated",
    "",
    "id",
    "progressUpdated",
    "totalProgress",
    "totalDownloaded",
    "totalSize",
    "speed",
    "eta",
    "chunkProgressUpdated",
    "std::vector<ChunkProgress>",
    "chunks",
    "downloadFinished",
    "success",
    "message",
    "downloadPaused",
    "downloadId",
    "statusChanged",
    "status",
    "startDownload",
    "url",
    "outputPath",
    "pauseDownload",
    "resumeDownload",
    "cancelDownload",
    "setSpeedLimit",
    "limit",
    "performWork",
    "updateProgress",
    "attemptNetworkRecovery"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_DownloadWorker[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   98,    2, 0x06,    1 /* Public */,
       4,    5,  101,    2, 0x06,    3 /* Public */,
      10,    1,  112,    2, 0x06,    9 /* Public */,
      13,    2,  115,    2, 0x06,   11 /* Public */,
      16,    1,  120,    2, 0x06,   14 /* Public */,
      18,    1,  123,    2, 0x06,   16 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      20,    2,  126,    2, 0x0a,   18 /* Public */,
      23,    0,  131,    2, 0x0a,   21 /* Public */,
      24,    1,  132,    2, 0x0a,   22 /* Public */,
      25,    0,  135,    2, 0x0a,   24 /* Public */,
      26,    1,  136,    2, 0x0a,   25 /* Public */,
      28,    0,  139,    2, 0x08,   27 /* Private */,
      29,    0,  140,    2, 0x08,   28 /* Private */,
      30,    0,  141,    2, 0x08,   29 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,    6,    7,    8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   14,   15,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, QMetaType::QString,   19,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   21,   22,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   27,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DownloadWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DownloadWorker.offsetsAndSizes,
    qt_meta_data_DownloadWorker,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_DownloadWorker_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DownloadWorker, std::true_type>,
        // method 'downloadIDGenerated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'progressUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'chunkProgressUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<ChunkProgress> &, std::false_type>,
        // method 'downloadFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadPaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'startDownload'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'pauseDownload'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'resumeDownload'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cancelDownload'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setSpeedLimit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'performWork'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'attemptNetworkRecovery'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DownloadWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DownloadWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->downloadIDGenerated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->progressUpdated((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5]))); break;
        case 2: _t->chunkProgressUpdated((*reinterpret_cast< std::add_pointer_t<std::vector<ChunkProgress>>>(_a[1]))); break;
        case 3: _t->downloadFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->downloadPaused((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->statusChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->startDownload((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->pauseDownload(); break;
        case 8: _t->resumeDownload((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->cancelDownload(); break;
        case 10: _t->setSpeedLimit((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 11: _t->performWork(); break;
        case 12: _t->updateProgress(); break;
        case 13: _t->attemptNetworkRecovery(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DownloadWorker::*)(QString );
            if (_t _q_method = &DownloadWorker::downloadIDGenerated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DownloadWorker::*)(double , double , double , double , double );
            if (_t _q_method = &DownloadWorker::progressUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DownloadWorker::*)(const std::vector<ChunkProgress> & );
            if (_t _q_method = &DownloadWorker::chunkProgressUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DownloadWorker::*)(bool , const QString & );
            if (_t _q_method = &DownloadWorker::downloadFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DownloadWorker::*)(const QString & );
            if (_t _q_method = &DownloadWorker::downloadPaused; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DownloadWorker::*)(const QString & );
            if (_t _q_method = &DownloadWorker::statusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *DownloadWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DownloadWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DownloadWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DownloadWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void DownloadWorker::downloadIDGenerated(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DownloadWorker::progressUpdated(double _t1, double _t2, double _t3, double _t4, double _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DownloadWorker::chunkProgressUpdated(const std::vector<ChunkProgress> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DownloadWorker::downloadFinished(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DownloadWorker::downloadPaused(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DownloadWorker::statusChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
