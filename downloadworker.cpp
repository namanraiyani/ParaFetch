#include "downloadworker.h"
#include "downloadmanager.h"
#include "httphelper.h"
#include <QDebug>
#include <QDir>
#include <QUuid>
#include <cmath>

DownloadWorker::DownloadWorker(QObject *parent)
    : QObject(parent), m_multiHandle(nullptr), m_fileSize(-1), 
      m_numChunks(0), m_supportsRanges(false), m_speedLimit(0), m_bytesAtStart(0), // Init
      m_userPaused(false), m_cancelled(false), m_isNetworkError(false)
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    m_workTimer = new QTimer(this);
    connect(m_workTimer, &QTimer::timeout, this, &DownloadWorker::performWork);
    
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &DownloadWorker::updateProgress);

    m_networkRetryTimer = new QTimer(this);
    connect(m_networkRetryTimer, &QTimer::timeout, this, &DownloadWorker::attemptNetworkRecovery);
}

DownloadWorker::~DownloadWorker() {
    cleanup();
    curl_global_cleanup();
}

int DownloadWorker::calculateOptimalConnections(curl_off_t size) {
    if (size <= 0) return 1;
    long long sizeMB = size / (1024 * 1024);
    int optimal = 1 + (sizeMB / 50); 
    if (optimal > 8) optimal = 8;
    return optimal;
}

bool DownloadWorker::probeFileInfo() {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    m_headers.clear();
    curl_easy_setopt(curl, CURLOPT_URL, m_url.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HttpHelper::headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &m_headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        char *finalUrl = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &finalUrl);
        if (finalUrl) m_url = QString::fromUtf8(finalUrl);
        m_fileSize = HttpHelper::getContentLength(m_headers);
        m_supportsRanges = HttpHelper::supportsRanges(m_headers);
        m_filename = HttpHelper::extractFilename(m_url, m_headers);
    }
    curl_easy_cleanup(curl);
    return res == CURLE_OK && m_fileSize > 0;
}

void DownloadWorker::startDownload(const QString& url, const QString& outputPath) {
    m_url = url;
    m_outputPath = outputPath;
    m_downloadId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    emit downloadIDGenerated(m_downloadId); 

    m_userPaused = false;
    m_cancelled = false;
    m_isNetworkError = false;
    m_bytesAtStart = 0; // Fresh download
    
    emit statusChanged("Connecting...");
    if (!probeFileInfo()) {
        emit downloadFinished(false, "Could not connect to server.");
        return;
    }
    
    m_numChunks = m_supportsRanges ? calculateOptimalConnections(m_fileSize) : 1;
    if (!initializeDownload(m_url, m_numChunks)) {
        emit downloadFinished(false, "Initialization failed");
        return;
    }
    
    DownloadManager::saveState(m_downloadId, m_url, m_outputPath, m_filename, m_numChunks, m_fileSize);
    
    m_globalStartTime = std::chrono::steady_clock::now();
    emit statusChanged(QString("Downloading with %1 connections...").arg(m_numChunks));
    m_progressTimer->start(200);
    m_workTimer->start(0);
}

bool DownloadWorker::initializeDownload(const QString& url, int numChunks) {
    m_chunks.clear();
    m_chunks.resize(numChunks);
    curl_off_t chunkSize = m_fileSize / numChunks;
    
    for (int i = 0; i < numChunks; ++i) {
        m_chunks[i].id = i + 1;
        m_chunks[i].start = i * chunkSize;
        m_chunks[i].end = (i == numChunks - 1) ? m_fileSize - 1 : (i + 1) * chunkSize - 1;
        m_chunks[i].size = m_chunks[i].end - m_chunks[i].start + 1;
        m_chunks[i].downloaded = 0;
        m_chunks[i].completed = false;
        m_chunks[i].filename = DownloadManager::getChunkFile(m_downloadId, m_chunks[i].id);
        m_chunks[i].lastUpdate = std::chrono::steady_clock::now();
        m_chunks[i].file = fopen(m_chunks[i].filename.toLocal8Bit().constData(), "wb");
        if (!m_chunks[i].file) { cleanup(); return false; }
    }

    m_multiHandle = curl_multi_init();
    curl_multi_setopt(m_multiHandle, CURLMOPT_MAX_TOTAL_CONNECTIONS, numChunks);
    curl_multi_setopt(m_multiHandle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    for (int i = 0; i < numChunks; ++i) {
        CURL* eh = curl_easy_init();
        m_easyHandles.push_back(eh);
        QString range = QString("%1-%2").arg(m_chunks[i].start).arg(m_chunks[i].end);
        curl_easy_setopt(eh, CURLOPT_URL, url.toUtf8().constData());
        if(m_supportsRanges) curl_easy_setopt(eh, CURLOPT_RANGE, range.toUtf8().constData());
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(eh, CURLOPT_WRITEDATA, &m_chunks[i]);
        curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT, 10L);
        
        // Apply speed limit if set
        if (m_speedLimit > 0) {
            curl_off_t limitPerHandle = (curl_off_t)(m_speedLimit / numChunks);
            curl_easy_setopt(eh, CURLOPT_MAX_RECV_SPEED_LARGE, limitPerHandle);
        }
        
        curl_multi_add_handle(m_multiHandle, eh);
    }
    return true;
}

void DownloadWorker::performWork() {
    if (!m_multiHandle || m_userPaused || m_cancelled || m_isNetworkError) return;
    
    int stillRunning = 0;
    CURLMcode mc = curl_multi_perform(m_multiHandle, &stillRunning);
    
    if (mc != CURLM_OK) {
        m_isNetworkError = true;
        m_workTimer->stop();
        m_networkRetryTimer->start(3000); 
        emit statusChanged("Network lost. Retrying...");
        return;
    }

    int msgsLeft;
    CURLMsg* msg;
    while ((msg = curl_multi_info_read(m_multiHandle, &msgsLeft))) {
        if (msg->msg == CURLMSG_DONE) {
            if (msg->data.result != CURLE_OK && msg->data.result != CURLE_PARTIAL_FILE) {
                m_isNetworkError = true;
                m_workTimer->stop();
                m_networkRetryTimer->start(3000);
                emit statusChanged("Connection dropped. Retrying...");
                return;
            }
        }
    }
    
    if (stillRunning == 0) {
        curl_off_t totalDownloaded = 0;
        for(const auto& c : m_chunks) totalDownloaded += c.downloaded;
        
        if (totalDownloaded < m_fileSize) {
             m_isNetworkError = true;
             m_workTimer->stop();
             m_networkRetryTimer->start(3000);
             emit statusChanged("Stream stalled. Retrying...");
             return;
        }

        m_workTimer->stop();
        m_progressTimer->stop();
        emit statusChanged("Merging files...");
        
        for (auto& chunk : m_chunks) { if (chunk.file) fclose(chunk.file); chunk.file = nullptr; }
        
        QString finalPath;
        if (DownloadManager::mergeChunks(m_downloadId, m_outputPath, m_numChunks, finalPath)) {
             QString targetPath = QDir(m_outputPath).filePath(m_filename);
             if (QFile::exists(targetPath)) QFile::remove(targetPath);
             QFile::rename(finalPath, targetPath);
             DownloadManager::cleanupChunks(m_downloadId, m_numChunks);
             emit downloadFinished(true, "Completed");
        } else {
             emit downloadFinished(false, "Merge Error");
        }
    }
}

void DownloadWorker::pauseDownload() {
    m_userPaused = true;
    m_workTimer->stop();
    m_networkRetryTimer->stop();
    
    for (auto& chunk : m_chunks) if (chunk.file) fflush(chunk.file);
    
    DownloadManager::saveState(m_downloadId, m_url, m_outputPath, m_filename, m_numChunks, m_fileSize);
    emit downloadPaused(m_downloadId);
    
    curl_off_t totalDownloaded = 0;
    for(const auto& c : m_chunks) totalDownloaded += c.downloaded;
    double progress = m_fileSize > 0 ? (double)totalDownloaded / m_fileSize : 0;
    
    // UI: Pause Speed 0
    emit progressUpdated(progress, totalDownloaded, m_fileSize, 0, 0); 
    emit statusChanged("Paused");
    
    cleanup(); 
}

void DownloadWorker::resumeDownload(const QString& downloadId) {
    m_downloadId = downloadId;
    
    QString url, outPath, fname;
    curl_off_t fsize;
    int chunks;
    if (!DownloadManager::loadState(downloadId, url, outPath, fname, chunks, fsize)) {
        emit downloadFinished(false, "Resume failed: State missing");
        return;
    }
    
    m_url = url; m_outputPath = outPath; m_filename = fname; m_numChunks = chunks; m_fileSize = fsize;
    
    m_chunks.clear();
    m_chunks.resize(chunks); 
    m_easyHandles.clear();
    
    m_bytesAtStart = 0; // Reset accumulator

    curl_off_t chunkSize = m_fileSize / chunks;
    for(int i=0; i<chunks; ++i) {
        m_chunks[i].id = i+1;
        m_chunks[i].start = i * chunkSize;
        m_chunks[i].end = (i == chunks - 1) ? m_fileSize - 1 : (i + 1) * chunkSize - 1;
        m_chunks[i].size = m_chunks[i].end - m_chunks[i].start + 1;
        m_chunks[i].filename = DownloadManager::getChunkFile(m_downloadId, i+1);
        
        QFile f(m_chunks[i].filename);
        m_chunks[i].downloaded = f.exists() ? f.size() : 0;
        
        // --- ACCUMULATE EXISTING BYTES ---
        m_bytesAtStart += m_chunks[i].downloaded;

        m_chunks[i].file = fopen(m_chunks[i].filename.toLocal8Bit().constData(), "ab");
        if(!m_chunks[i].file) { emit downloadFinished(false, "File access error"); return; }
    }
    
    m_multiHandle = curl_multi_init();
    curl_multi_setopt(m_multiHandle, CURLMOPT_MAX_TOTAL_CONNECTIONS, chunks);
    curl_multi_setopt(m_multiHandle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
    
    for(int i=0; i<chunks; ++i) {
        if(m_chunks[i].downloaded >= m_chunks[i].size) continue;

        CURL* eh = curl_easy_init();
        m_easyHandles.push_back(eh);
        curl_off_t currentPos = m_chunks[i].start + m_chunks[i].downloaded;
        QString range = QString("%1-%2").arg(currentPos).arg(m_chunks[i].end);
        
        curl_easy_setopt(eh, CURLOPT_URL, m_url.toUtf8().constData());
        curl_easy_setopt(eh, CURLOPT_RANGE, range.toUtf8().constData());
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(eh, CURLOPT_WRITEDATA, &m_chunks[i]);
        curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
        
        // Apply speed limit if set
        if (m_speedLimit > 0) {
            curl_off_t limitPerHandle = (curl_off_t)(m_speedLimit / chunks);
            curl_easy_setopt(eh, CURLOPT_MAX_RECV_SPEED_LARGE, limitPerHandle);
        }
        
        curl_multi_add_handle(m_multiHandle, eh);
    }

    m_userPaused = false;
    m_isNetworkError = false;
    
    // Reset timer
    m_globalStartTime = std::chrono::steady_clock::now();
    
    m_workTimer->start(0);
    m_progressTimer->start(200);
    emit statusChanged("Resumed");
}

void DownloadWorker::cleanup() {
    m_workTimer->stop();
    m_progressTimer->stop();
    m_networkRetryTimer->stop();
    if (m_multiHandle) {
        for (auto h : m_easyHandles) { curl_multi_remove_handle(m_multiHandle, h); curl_easy_cleanup(h); }
        curl_multi_cleanup(m_multiHandle);
        m_multiHandle = nullptr;
        m_easyHandles.clear();
    }
    for (auto& chunk : m_chunks) { if (chunk.file) fclose(chunk.file); chunk.file = nullptr; }
}

size_t DownloadWorker::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realSize = size * nmemb;
    ChunkData* chunk = static_cast<ChunkData*>(userp);
    if (!chunk || !chunk->file) return 0;
    size_t written = fwrite(contents, 1, realSize, chunk->file);
    if (written == realSize) {
        chunk->downloaded += written;
        chunk->lastUpdate = std::chrono::steady_clock::now();
    }
    return written;
}

void DownloadWorker::attemptNetworkRecovery() {
    // Network recovery logic goes here (omitted for brevity)
}

void DownloadWorker::updateProgress() {
    if(m_userPaused) return;

    QMutexLocker locker(&m_chunkMutex);
    curl_off_t totalDownloaded = 0;
    std::vector<ChunkProgress> cProgs;
    
    for(const auto& c : m_chunks) {
        totalDownloaded += c.downloaded;
        ChunkProgress cp;
        cp.id = c.id;
        cp.downloaded = c.downloaded;
        cp.size = c.size;
        cp.startOffset = c.start;
        cp.totalFileSize = m_fileSize;
        cProgs.push_back(cp);
    }
    
    // --- CORRECTED ETA CALCULATION ---
    // 1. Bytes downloaded THIS SESSION = Total on disk - Bytes present at start
    curl_off_t sessionBytes = totalDownloaded - m_bytesAtStart;
    if (sessionBytes < 0) sessionBytes = 0; // Safety

    // 2. Time elapsed THIS SESSION
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_globalStartTime).count() / 1000.0;
    
    // 3. Speed = SessionBytes / SessionTime
    double speed = (elapsed > 0.1) ? (double)sessionBytes / elapsed : 0.0;
    
    // 4. ETA = Remaining / Current Speed
    curl_off_t remaining = m_fileSize - totalDownloaded;
    double eta = (speed > 0) ? (double)remaining / speed : 0.0;
    double progress = m_fileSize > 0 ? (double)totalDownloaded / m_fileSize : 0;
    
    emit progressUpdated(progress, totalDownloaded, m_fileSize, speed, eta);
    emit chunkProgressUpdated(cProgs);
}

void DownloadWorker::cancelDownload() {
    m_cancelled = true;
    cleanup();
    DownloadManager::cleanupChunks(m_downloadId, m_numChunks);
    emit downloadFinished(false, "Cancelled");
}

void DownloadWorker::setSpeedLimit(double limit) {
    m_speedLimit = limit;
    if (m_multiHandle && !m_easyHandles.empty()) {
        curl_off_t limitPerHandle = (limit > 0 && m_numChunks > 0) ? (curl_off_t)(limit / m_numChunks) : 0;
        for (auto eh : m_easyHandles) {
            curl_easy_setopt(eh, CURLOPT_MAX_RECV_SPEED_LARGE, limitPerHandle);
        }
    }
}