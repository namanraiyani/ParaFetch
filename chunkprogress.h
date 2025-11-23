#ifndef CHUNKPROGRESS_H
#define CHUNKPROGRESS_H
#include <QString>
struct ChunkProgress {
    int id;
    double downloaded;
    double size;
    double startOffset;
    double totalFileSize; 
    QString status;
};
#endif