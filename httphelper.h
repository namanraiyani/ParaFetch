#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include <QString>
#include <QMap>
#include <curl/curl.h>

class HttpHelper {
public:
    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata);
    static QString extractFilename(const QString& url, const QMap<QString, QString>& headers);
    static bool supportsRanges(const QMap<QString, QString>& headers);
    static curl_off_t getContentLength(const QMap<QString, QString>& headers);
};

#endif