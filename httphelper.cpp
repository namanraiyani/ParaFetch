#include "httphelper.h"
#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>

size_t HttpHelper::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t realSize = size * nitems;
    QMap<QString, QString>* headers = static_cast<QMap<QString, QString>*>(userdata);
    
    QString header = QString::fromUtf8(buffer, realSize).trimmed();
    int colonPos = header.indexOf(':');
    if (colonPos > 0) {
        QString key = header.left(colonPos).trimmed().toLower();
        QString value = header.mid(colonPos + 1).trimmed();
        headers->insert(key, value);
    }
    
    return realSize;
}

QString HttpHelper::extractFilename(const QString& url, const QMap<QString, QString>& headers) {
    if (headers.contains("content-disposition")) {
        QString disposition = headers["content-disposition"];
        QRegularExpression re(R"(filename[*]?=(?:UTF-8'')?["\']?([^"\';\r\n]+)["\']?)", 
                            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(disposition);
        if (match.hasMatch()) {
            QString filename = match.captured(1).trimmed();
            return QUrl::fromPercentEncoding(filename.toUtf8());
        }
    }
    
    QUrl qurl(url);
    QString path = qurl.path();
    QString filename = QFileInfo(path).fileName();
    
    if (!filename.isEmpty() && filename.contains('.')) {
        return filename;
    }
    
    if (headers.contains("content-type")) {
        QString contentType = headers["content-type"].toLower();
        if (contentType.contains("pdf")) return "download.pdf";
        if (contentType.contains("zip")) return "download.zip";
        if (contentType.contains("video")) return "download.mp4";
        if (contentType.contains("audio")) return "download.mp3";
        if (contentType.contains("image")) return "download.jpg";
    }
    
    return filename.isEmpty() ? "download.bin" : filename;
}

bool HttpHelper::supportsRanges(const QMap<QString, QString>& headers) {
    if (headers.contains("accept-ranges")) {
        QString value = headers["accept-ranges"].toLower();
        return value != "none";
    }
    return false;
}

curl_off_t HttpHelper::getContentLength(const QMap<QString, QString>& headers) {
    if (headers.contains("content-length")) {
        return headers["content-length"].toLongLong();
    }
    return -1;
}