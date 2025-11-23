#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QString>
#include <QMap>
#include <QMutex>
#include <QFile>
#include <curl/curl.h>

class DownloadManager {
public:
    static QString getTempDirectory();
    static QString getStateFile(const QString& downloadId);
    static QString getChunkFile(const QString& downloadId, int chunkId);
    static bool saveState(const QString& downloadId, const QString& url, 
                         const QString& outputPath, const QString& filename,
                         int numChunks, curl_off_t fileSize);
    static bool loadState(const QString& downloadId, QString& url, 
                         QString& outputPath, QString& filename,
                         int& numChunks, curl_off_t& fileSize);
    static bool deleteState(const QString& downloadId);
    static bool mergeChunks(const QString& downloadId, const QString& outputPath, 
                           int numChunks, QString& finalPath);
    static void cleanupChunks(const QString& downloadId, int numChunks);
};

#endif