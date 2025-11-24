#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMap>
#include <curl/curl.h>
#include <vector>
#include <atomic>
#include <chrono>
#include "chunkprogress.h"

struct ChunkData {
    int id;
    QString filename;
    FILE* file;
    curl_off_t start;
    curl_off_t end;
    curl_off_t size;
    curl_off_t downloaded;
    bool completed;
    std::chrono::steady_clock::time_point lastUpdate;
};

class DownloadWorker : public QObject {
    Q_OBJECT
public:
    explicit DownloadWorker(QObject *parent = nullptr);
    ~DownloadWorker();

public slots:
    void startDownload(const QString& url, const QString& outputPath);
    void pauseDownload();
    void resumeDownload(const QString& downloadId);
    void cancelDownload();
    void setSpeedLimit(double limit); // limit in bytes/sec, 0 = unlimited

private slots:
    void performWork();
    void updateProgress();
    void attemptNetworkRecovery();

signals:
    void downloadIDGenerated(QString id); 
    void progressUpdated(double totalProgress, double totalDownloaded, double totalSize, double speed, double eta);
    void chunkProgressUpdated(const std::vector<ChunkProgress>& chunks);
    void downloadFinished(bool success, const QString& message);
    void downloadPaused(const QString& downloadId);
    void statusChanged(const QString& status);

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    bool initializeDownload(const QString& url, int numChunks);
    void cleanup();
    bool probeFileInfo();
    int calculateOptimalConnections(curl_off_t size);
    
    std::vector<ChunkData> m_chunks;
    std::vector<CURL*> m_easyHandles;
    CURLM* m_multiHandle;
    
    QString m_url;
    QString m_outputPath;
    QString m_filename;
    QString m_downloadId;
    curl_off_t m_fileSize;
    int m_numChunks;
    bool m_supportsRanges;
    double m_speedLimit; // Bytes per second
    
    // --- NEW: Track bytes present when session started ---
    curl_off_t m_bytesAtStart; 

    std::atomic<bool> m_userPaused;
    std::atomic<bool> m_cancelled;
    bool m_isNetworkError;
    
    std::chrono::steady_clock::time_point m_globalStartTime;
    QTimer* m_workTimer;
    QTimer* m_progressTimer;
    QTimer* m_networkRetryTimer;
    QMutex m_chunkMutex;
    QMap<QString, QString> m_headers;
};
#endif