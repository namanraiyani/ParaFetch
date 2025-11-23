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
#include "downloadworker.h"

// --- Custom Widget: Segmented Progress Bar with Text ---
class TableSegmentedBar : public QWidget
{
    Q_OBJECT
public:
    TableSegmentedBar(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_progress = 0;
        m_totalSize = 0;
        m_downloaded = 0;
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
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        p.setBrush(QColor(36, 40, 59));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 4, 4);

        if (!m_chunks.empty() && m_totalSize > 0)
        {
            double scale = (double)width() / m_totalSize;
            p.setBrush(QColor(115, 218, 202));
            for (const auto &chunk : m_chunks)
            {
                double x = chunk.startOffset * scale;
                double w = chunk.downloaded * scale;
                if (w > 0)
                {
                    QRectF chunkRect(x, 0, w, height());
                    p.drawRect(chunkRect);
                }
            }
        }
        else if (m_progress > 0)
        {
            p.setBrush(QColor(115, 218, 202));
            p.drawRoundedRect(0, 0, width() * m_progress, height(), 4, 4);
        }

        p.setPen(Qt::white);
        QFont font("Segoe UI", 9, QFont::Bold);
        p.setFont(font);
        QString text = QString::number(m_progress * 100, 'f', 1) + "%";
        p.drawText(rect(), Qt::AlignCenter, text);
    }

private:
    double m_progress;
    double m_downloaded;
    double m_totalSize;
    std::vector<ChunkProgress> m_chunks;
};

// --- Global Speed Graph (Unchanged) ---
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

        if (m_history.isEmpty())
            return;

        double wStep = (double)width() / (m_history.size() - 1);
        double h = height();
        double scaling = (m_maxSpeed > 0) ? (h / (m_maxSpeed * 1.1)) : 0;

        QPainterPath path;
        path.moveTo(0, h);
        for (int i = 0; i < m_history.size(); ++i)
        {
            path.lineTo(i * wStep, h - (m_history[i] * scaling));
        }
        path.lineTo(width(), h);
        path.closeSubpath();

        QLinearGradient grad(0, 0, 0, h);
        grad.setColorAt(0, QColor(122, 162, 247, 100));
        grad.setColorAt(1, QColor(122, 162, 247, 0));
        p.fillPath(path, grad);

        QPainterPath linePath;
        linePath.moveTo(0, h - (m_history[0] * scaling));
        for (int i = 1; i < m_history.size(); ++i)
        {
            linePath.lineTo(i * wStep, h - (m_history[i] * scaling));
        }
        p.setPen(QPen(QColor(122, 162, 247), 2));
        p.drawPath(linePath);
    }

private:
    QVector<double> m_history;
    double m_maxSpeed = 1024.0;
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
};

class MyForm : public QMainWindow
{
    Q_OBJECT
public:
    MyForm(QWidget *parent = nullptr);
    ~MyForm();

private slots:
    void onAddClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onRemoveClicked();

    void onWorkerProgress(QString id, double prog, double dl, double total, double speed, double eta);
    void onWorkerChunkProgress(QString id, const std::vector<ChunkProgress> &chunks);
    void onWorkerStatus(QString id, QString status);
    void onWorkerFinished(QString id, bool success, QString msg);
    void onWorkerIDGenerated(QString uid, QString downloadId);

    void updateGlobalStats();

private:
    void setupUI();
    void applyStyles();
    QString formatSize(double bytes);
    QString formatTime(double seconds);

    QTableWidget *table;
    GlobalSpeedGraph *globalGraph;
    QLabel *lblGlobalSpeed;

    QMap<QString, TaskInfo *> tasks;
    QTimer *globalTimer;
};

#endif