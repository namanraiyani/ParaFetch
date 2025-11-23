#include "downloadmanager.h"
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QFile>

QString DownloadManager::getTempDirectory()
{
    QString p = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/parafetch";
    QDir().mkpath(p);
    return p;
}

QString DownloadManager::getStateFile(const QString& id) { return getTempDirectory() + "/" + id + ".state"; }
QString DownloadManager::getChunkFile(const QString& id, int c) { return getTempDirectory() + "/" + id + ".part" + QString::number(c); }

bool DownloadManager::saveState(const QString& id, const QString& url,
                            const QString& outPath, const QString& name,
                            int chunks, curl_off_t size)
{
    QFile f(getStateFile(id));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&f);
    out << url << "\n" << outPath << "\n" << name << "\n" << chunks << "\n" << size;
    return true;
}

bool DownloadManager::loadState(const QString& id, QString& url,
                                QString& outPath, QString& name,
                                int& chunks, curl_off_t& size)
{
    QFile f(getStateFile(id));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&f);
    url = in.readLine();
    outPath = in.readLine();
    name = in.readLine();
    chunks = in.readLine().toInt();
    size = in.readLine().toLongLong();
    return true;
}

bool DownloadManager::deleteState(const QString& id) { return QFile::remove(getStateFile(id)); }

bool DownloadManager::mergeChunks(const QString& id, const QString& outPath, int chunks, QString& finalPath)
{
    QDir dir(outPath);
    finalPath = dir.absoluteFilePath(id + ".downloaded");

    QFile out(finalPath);
    if (!out.open(QIODevice::WriteOnly)) return false;

    for (int i = 1; i <= chunks; ++i) {
        QFile in(getChunkFile(id, i));
        if (!in.open(QIODevice::ReadOnly)) {
            out.close();
            QFile::remove(finalPath);
            return false;
        }
        out.write(in.readAll());
        in.close();
    }
    out.close();
    cleanupChunks(id, chunks);
    return true;
}

void DownloadManager::cleanupChunks(const QString& id, int chunks)
{
    for (int i = 1; i <= chunks; ++i) QFile::remove(getChunkFile(id, i));
    deleteState(id);
}