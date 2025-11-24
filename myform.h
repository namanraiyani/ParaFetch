#ifndef MYFORM_H
#define MYFORM_H

#include <QMainWindow>
#include <QTableWidget>
#include <QToolBar>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMap>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QHeaderView>
#include <QStorageInfo>
#include <QDir>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <vector> 
#include "downloadworker.h"
#include "settingsdialog.h"
#include "batchdownloaddialog.h"
#include "notificationmanager.h"

// --- Custom Widget: Segmented Progress Bar with Text Overlay ---
class TableSegmentedBar : public QWidget
{
    Q_OBJECT
public:
    TableSegmentedBar(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_progress = 0;
        m_totalSize = 0;
        m_downloaded = 0;
        // Enforce a minimum size so the layout allocates space
        setMinimumHeight(26);
    }

    void setProgressData(double dl, double total)
    {
        m_downloaded = dl;
        m_totalSize = total;
        m_progress = (total > 0) ? (dl / total) : 0;
        update();
    }

    void setChunks(const std::vector<ChunkProgress> &chunks)
    {
        m_chunks = chunks;
        // Optional: Recalculate progress from chunks to ensure text sync
        if (m_totalSize > 0 && !chunks.empty()) {
            double totalDownloaded = 0;
            for(const auto& c : chunks) totalDownloaded += c.downloaded;
            m_progress = totalDownloaded / m_totalSize;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 1. Draw Background (Darker Surface)
        p.setBrush(QColor(44, 44, 46));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 4, 4);

        // 2. Draw Progress Bar (Solid or Chunks)
        if (!m_chunks.empty() && m_totalSize > 0)
        {
            double scale = (double)width() / m_totalSize;
            p.setBrush(QColor(10, 132, 255)); // MacOS Blue
            for (const auto &chunk : m_chunks)
            {
                double x = chunk.startOffset * scale;
                double w = chunk.downloaded * scale;
                if (w < 1.0 && w > 0.0) w = 1.0; 
                
                if (w > 0) {
                    QRectF chunkRect(x, 0, w, height());
                    p.drawRect(chunkRect);
                }
            }
        }
        else if (m_progress > 0)
        {
            p.setBrush(QColor(10, 132, 255));
            p.drawRoundedRect(0, 0, width() * m_progress, height(), 4, 4);
        }

        // 3. Draw Text Overlay ("X %")
        p.setBrush(Qt::NoBrush);
        
        QString text = QString::number(m_progress * 100, 'f', 1) + " %";
        
        QFont font = p.font();
        font.setPixelSize(11); 
        font.setBold(true);
        p.setFont(font);
        
        QRect r = rect();

        // Draw Main Text (White)
        p.setPen(QColor(255, 255, 255));
        p.drawText(r, Qt::AlignCenter, text);
    }

private:
    double m_progress;
    double m_downloaded;
    double m_totalSize;
    std::vector<ChunkProgress> m_chunks;
};

// --- Global Speed Graph with Gradient Fade ---
class GlobalSpeedGraph : public QWidget
{
    Q_OBJECT
public:
    GlobalSpeedGraph(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(80);
        m_history.fill(0, 200);
        setStyleSheet("background-color: transparent;");
    }
    
    void addPoint(double speed)
    {
        m_history.pop_front();
        m_history.push_back(speed);
        m_maxSpeed = std::max(m_maxSpeed, speed);
        
        if (m_maxSpeed > speed * 2 && m_maxSpeed > 1024 * 1024)
            m_maxSpeed *= 0.98;
        if (m_maxSpeed < 1024)
            m_maxSpeed = 1024;
            
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        if (m_history.isEmpty()) return;

        double wStep = (double)width() / (m_history.size() - 1);
        double h = height();
        double scaling = (m_maxSpeed > 0) ? (h / (m_maxSpeed * 1.1)) : 0;

        QPainterPath path;
        path.moveTo(0, h);
        for (int i = 0; i < m_history.size(); ++i) {
            path.lineTo(i * wStep, h - (m_history[i] * scaling));
        }
        path.lineTo(width(), h);
        path.closeSubpath();

        QLinearGradient fadeGrad(0, 0, width(), 0);
        fadeGrad.setColorAt(0, QColor(28, 28, 30, 255));
        fadeGrad.setColorAt(0.3, QColor(10, 132, 255, 50));
        fadeGrad.setColorAt(1, QColor(175, 82, 222, 50));
        p.fillPath(path, fadeGrad);

        QPainterPath linePath;
        linePath.moveTo(0, h - (m_history[0] * scaling));
        for (int i = 1; i < m_history.size(); ++i) {
            linePath.lineTo(i * wStep, h - (m_history[i] * scaling));
        }
        
        QLinearGradient lineGrad(0, 0, width(), 0);
        lineGrad.setColorAt(0, QColor(10, 132, 255));
        lineGrad.setColorAt(0.5, QColor(90, 200, 250));
        lineGrad.setColorAt(1, QColor(175, 82, 222));
        
        QPen pen;
        pen.setWidth(2);
        pen.setBrush(lineGrad);
        p.setPen(pen);
        p.drawPath(linePath);
    }

private:
    QVector<double> m_history;
    double m_maxSpeed = 1024.0;
};

// --- Disk Usage Pie Chart Widget ---
class DiskUsagePieChart : public QWidget
{
    Q_OBJECT
public:
    DiskUsagePieChart(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(100, 100);
        setMaximumSize(100, 100);
        setStyleSheet("background-color: transparent;");
        updateDiskSpace();
        
        QTimer* diskTimer = new QTimer(this);
        connect(diskTimer, &QTimer::timeout, this, &DiskUsagePieChart::updateDiskSpace);
        diskTimer->start(5000);
    }
    
    void updateDiskSpace()
    {
        QStorageInfo storage(QDir::homePath());
        m_totalSpace = storage.bytesTotal();
        m_freeSpace = storage.bytesAvailable();
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        int pieSize = 70;
        int pieX = (width() - pieSize) / 2;
        int pieY = (height() - pieSize) / 2 + 5;
        
        if (m_totalSpace == 0) return;
        
        double freePercent = (double)m_freeSpace / m_totalSpace;
        double usedPercent = 1.0 - freePercent;
        int usedAngle = (int)(usedPercent * 360 * 16);
        
        p.setBrush(QColor(44, 44, 46));
        p.setPen(Qt::NoPen);
        p.drawEllipse(pieX - 5, pieY - 5, pieSize + 10, pieSize + 10);
        
        QConicalGradient usedGrad(pieX + pieSize/2, pieY + pieSize/2, 90);
        usedGrad.setColorAt(0, QColor(10, 132, 255));
        usedGrad.setColorAt(1, QColor(175, 82, 222));
        p.setBrush(usedGrad);
        p.drawPie(pieX, pieY, pieSize, pieSize, 90 * 16, -usedAngle);
        
        p.setBrush(QColor(48, 209, 88)); // Green
        p.drawPie(pieX, pieY, pieSize, pieSize, 90 * 16 - usedAngle, -(360 * 16 - usedAngle));
        
        p.setBrush(QColor(28, 28, 30));
        int innerSize = pieSize * 0.65;
        int innerOffset = (pieSize - innerSize) / 2;
        p.drawEllipse(pieX + innerOffset, pieY + innerOffset, innerSize, innerSize);
        
        p.setPen(QColor(229, 229, 234));
        QFont font("Inter", 9, QFont::Bold);
        p.setFont(font);
        QString percentText = QString::number((int)(freePercent * 100)) + "%";
        p.drawText(QRect(pieX, pieY + 15, pieSize, 20), Qt::AlignCenter, percentText);
        
        QFont freeFont("Inter", 6, QFont::Bold);
        p.setFont(freeFont);
        p.setPen(QColor(48, 209, 88));
        p.drawText(QRect(pieX, pieY + 30, pieSize, 15), Qt::AlignCenter, "FREE");
        
        p.setPen(QColor(142, 142, 147));
        QFont labelFont("Inter", 8, QFont::Bold);
        p.setFont(labelFont);
        p.drawText(QRect(0, 0, width(), 20), Qt::AlignCenter, "DISK USAGE");
    }

private:
    qint64 m_totalSpace = 0;
    qint64 m_freeSpace = 0;
};

class AddDownloadDialog : public QDialog
{
    Q_OBJECT
public:
    QLineEdit *urlEdit;
    QLineEdit *pathEdit;
    AddDownloadDialog(QWidget *parent = nullptr);
};

struct TaskInfo
{
    QThread *thread;
    DownloadWorker *worker;
    int tableRow;
    double currentSpeed;
    QString downloadId;
    QString url;
    QString outputPath;
};

class MyForm : public QMainWindow
{
    Q_OBJECT
public:
    MyForm(QWidget *parent = nullptr);
    ~MyForm();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onAddClicked();
    void onBatchAddClicked();
    void onSettingsClicked();
    void onPauseResumeToggle();
    void onRemoveClicked();
    void updatePauseResumeButton();
    void showContextMenu(const QPoint &pos);
    void onClipboardChanged();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void onWorkerProgress(QString id, double prog, double dl, double total, double speed, double eta);
    void onWorkerChunkProgress(QString id, const std::vector<ChunkProgress> &chunks);
    void onWorkerStatus(QString id, QString status);
    void onWorkerFinished(QString id, bool success, QString msg);
    void onWorkerIDGenerated(QString uid, QString downloadId);

    void updateGlobalStats();

private:
    void setupUI();
    void applyStyles();
    void loadSettings();
    void addDownload(const QString &url, const QString &path);
    void openDownloadFolder(int row);
    void openDownloadedFile(int row);
    void copyUrlToClipboard(int row);
    QString formatSize(double bytes);
    QString formatTime(double seconds);

    QTableWidget *table;
    GlobalSpeedGraph *globalGraph;
    QLabel *lblGlobalSpeed;
    QAction *actPauseResume;
    QSystemTrayIcon *trayIcon;
    QClipboard *clipboard;
    QSettings *settings;

    QMap<QString, TaskInfo *> tasks;
    QTimer *globalTimer;
    
    // Settings
    QString defaultDownloadPath;
    int defaultConnections;
    double defaultSpeedLimit;
    bool clipboardMonitoringEnabled;
    bool notificationsEnabled;
};

#endif