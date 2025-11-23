#include "myform.h"
#include <QHeaderView>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>
#include <QUrl>
#include <QFileInfo>
#include <cmath> // for isinf, isnan

// --- Add Download Dialog ---
AddDownloadDialog::AddDownloadDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Add New Download");
    resize(500, 150);
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QFormLayout* form = new QFormLayout();
    urlEdit = new QLineEdit();
    urlEdit->setPlaceholderText("https://example.com/file.iso");
    pathEdit = new QLineEdit(QDir::homePath() + "/Downloads");
    
    QPushButton* btnBrowse = new QPushButton("...");
    btnBrowse->setFixedWidth(30);
    connect(btnBrowse, &QPushButton::clicked, [=](){
        QString dir = QFileDialog::getExistingDirectory(this, "Select Folder", pathEdit->text());
        if(!dir.isEmpty()) pathEdit->setText(dir);
    });

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(btnBrowse);

    form->addRow("URL:", urlEdit);
    form->addRow("Save to:", pathLayout);
    layout->addLayout(form);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    
    setStyleSheet("background-color: #24283b; color: #c0caf5; QLineEdit { padding: 5px; }");
}

// --- Main Form Constructor ---
MyForm::MyForm(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    applyStyles();

    globalTimer = new QTimer(this);
    connect(globalTimer, &QTimer::timeout, this, &MyForm::updateGlobalStats);
    globalTimer->start(500); 
}

MyForm::~MyForm() {
    for(auto task : tasks) {
        if(task->thread && task->thread->isRunning()) {
            task->thread->quit();
            task->thread->wait();
        }
        delete task;
    }
}

// --- UI Setup ---
void MyForm::setupUI() {
    setWindowTitle("ParaFetch Manager");
    resize(1100, 650);

    QWidget* central = new QWidget();
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // Toolbar
    QToolBar* toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    
    QAction* actAdd = toolbar->addAction("+ Add Task");
    QAction* actPause = toolbar->addAction(" ⏸ Pause");
    QAction* actResume = toolbar->addAction(" ▶ Resume");
    QAction* actRemove = toolbar->addAction("x Remove");

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);

    connect(actAdd, &QAction::triggered, this, &MyForm::onAddClicked);
    connect(actPause, &QAction::triggered, this, &MyForm::onPauseClicked);
    connect(actResume, &QAction::triggered, this, &MyForm::onResumeClicked);
    connect(actRemove, &QAction::triggered, this, &MyForm::onRemoveClicked);

    // Table
    table = new QTableWidget();
    table->setColumnCount(7); 
    table->setHorizontalHeaderLabels({"Name", "Downloaded", "Size", "Progress", "Speed", "ETA", "Status"});
    
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    
    table->setColumnWidth(0, 250); // Name
    table->setColumnWidth(1, 100); // Downloaded
    table->setColumnWidth(2, 100); // Size
    table->setColumnWidth(3, 250); // Progress
    table->setColumnWidth(4, 100); // Speed
    table->setColumnWidth(5, 100); // ETA
    
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    mainLayout->addWidget(table);

    // Bottom Panel
    QFrame* bottomPanel = new QFrame();
    bottomPanel->setFixedHeight(120);
    bottomPanel->setObjectName("BottomPanel");
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomPanel);
    
    globalGraph = new GlobalSpeedGraph();
    lblGlobalSpeed = new QLabel("0.00 MB/s");
    lblGlobalSpeed->setStyleSheet("font-size: 24px; font-weight: bold; color: #7aa2f7;");

    QVBoxLayout* statsLayout = new QVBoxLayout();
    statsLayout->addWidget(new QLabel("TOTAL DOWNLOAD SPEED"));
    statsLayout->addWidget(lblGlobalSpeed);
    statsLayout->addStretch();

    bottomLayout->addLayout(statsLayout);
    bottomLayout->addWidget(globalGraph, 1); 

    mainLayout->addWidget(bottomPanel);
}

void MyForm::applyStyles() {
    setStyleSheet(R"(
        QMainWindow { background-color: #1a1b26; }
        QToolBar { background-color: #16161e; border-bottom: 1px solid #414868; padding: 5px; spacing: 10px; }
        QToolButton { background-color: #7aa2f7; color: #15161e; font-weight: bold; padding: 5px 15px; border-radius: 4px; }
        QToolButton:hover { background-color: #89ddff; }
        
        QTableWidget { background-color: #1a1b26; color: #a9b1d6; border: none; gridline-color: #414868; }
        QTableWidget::item { padding: 5px; border-bottom: 1px solid #24283b; }
        QTableWidget::item:selected { background-color: #2f3549; color: #fff; }
        QHeaderView::section { background-color: #24283b; color: #7aa2f7; padding: 5px; border: none; font-weight: bold; border-right: 1px solid #414868; }
        
        QFrame#BottomPanel { background-color: #16161e; border-top: 1px solid #414868; }
        QLabel { color: #a9b1d6; }
    )");
}

// --- Actions ---

void MyForm::onAddClicked() {
    AddDownloadDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        QString url = dlg.urlEdit->text().trimmed();
        QString path = dlg.pathEdit->text();
        if(url.isEmpty()) return;

        TaskInfo* task = new TaskInfo();
        task->thread = new QThread();
        task->worker = new DownloadWorker();
        task->worker->moveToThread(task->thread);
        task->currentSpeed = 0;

        int row = table->rowCount();
        table->insertRow(row);
        
        QString fileName = QFileInfo(QUrl(url).path()).fileName();
        if(fileName.isEmpty()) fileName = "downloading...";
        table->setItem(row, 0, new QTableWidgetItem(fileName)); 
        
        table->setItem(row, 1, new QTableWidgetItem("0 B"));
        table->setItem(row, 2, new QTableWidgetItem("--"));
        
        TableSegmentedBar* pBar = new TableSegmentedBar();
        table->setCellWidget(row, 3, pBar);

        table->setItem(row, 4, new QTableWidgetItem("0 B/s"));
        table->setItem(row, 5, new QTableWidgetItem("--"));
        
        QTableWidgetItem* statusItem = new QTableWidgetItem("Downloading");
        statusItem->setForeground(QColor("#e0af68"));
        table->setItem(row, 6, statusItem);
        
        task->tableRow = row;
        QString uid = QString::number((quintptr)task); 
        tasks.insert(uid, task);

        connect(task->thread, &QThread::started, task->worker, [=](){ 
            task->worker->startDownload(url, path); 
        });

        connect(task->worker, &DownloadWorker::progressUpdated, this, [=](double p, double dl, double tot, double spd, double eta){
            this->onWorkerProgress(uid, p, dl, tot, spd, eta);
        });

        connect(task->worker, &DownloadWorker::chunkProgressUpdated, this, [=](const std::vector<ChunkProgress>& c){
            this->onWorkerChunkProgress(uid, c);
        });

        connect(task->worker, &DownloadWorker::statusChanged, this, [=](QString s){
            this->onWorkerStatus(uid, s);
        });
        
        connect(task->worker, &DownloadWorker::downloadIDGenerated, this, [=](QString downId){
            this->onWorkerIDGenerated(uid, downId);
        });

        connect(task->worker, &DownloadWorker::downloadFinished, this, [=](bool s, QString m){
            this->onWorkerFinished(uid, s, m);
        });
        
        connect(task->worker, &DownloadWorker::downloadFinished, task->thread, &QThread::quit);
        connect(task->thread, &QThread::finished, task->worker, &QObject::deleteLater);

        task->thread->start();
    }
}

void MyForm::onWorkerIDGenerated(QString uid, QString downloadId) {
    if(tasks.contains(uid)) {
        tasks[uid]->downloadId = downloadId;
    }
}

void MyForm::onWorkerProgress(QString id, double prog, double dl, double total, double speed, double eta) {
    if(!tasks.contains(id)) return;
    TaskInfo* t = tasks[id];
    int row = t->tableRow;
    t->currentSpeed = speed; 

    TableSegmentedBar* pBar = qobject_cast<TableSegmentedBar*>(table->cellWidget(row, 3));
    if(pBar) pBar->setProgressData(dl, total); 

    table->item(row, 1)->setText(formatSize(dl));
    table->item(row, 2)->setText(formatSize(total));
    table->item(row, 4)->setText(formatSize(speed) + "/s");
    table->item(row, 5)->setText(formatTime(eta));
}

void MyForm::onWorkerChunkProgress(QString id, const std::vector<ChunkProgress>& chunks) {
    if(!tasks.contains(id)) return;
    int row = tasks[id]->tableRow;
    
    TableSegmentedBar* pBar = qobject_cast<TableSegmentedBar*>(table->cellWidget(row, 3));
    if(pBar) pBar->setChunks(chunks);
}

void MyForm::onWorkerStatus(QString id, QString status) {
    if(!tasks.contains(id)) return;
    int row = tasks[id]->tableRow;
    QTableWidgetItem* item = table->item(row, 6);
    
    QString displayStatus;
    QColor displayColor;

    if (status.contains("Paused", Qt::CaseInsensitive)) {
        displayStatus = "Paused";
        displayColor = QColor("#7aa2f7"); // Blue
    } 
    else if (status.contains("Complete", Qt::CaseInsensitive)) {
        displayStatus = "Completed";
        displayColor = QColor("#9ece6a"); // Green
    } 
    else if (status.contains("Error", Qt::CaseInsensitive) || 
             status.contains("Failed", Qt::CaseInsensitive) || 
             status.contains("Cancelled", Qt::CaseInsensitive)) {
        displayStatus = "Error";
        displayColor = QColor("#f7768e"); // Red
    } 
    else {
        displayStatus = "Downloading";
        displayColor = QColor("#e0af68"); // Yellow
    }

    item->setText(displayStatus);
    item->setForeground(displayColor);
}

void MyForm::onWorkerFinished(QString id, bool success, QString msg) {
    if(!tasks.contains(id)) return;
    TaskInfo* t = tasks[id];
    t->currentSpeed = 0; 
    
    QTableWidgetItem* statusItem = table->item(t->tableRow, 6);
    
    if (success) {
        statusItem->setText("Completed");
        statusItem->setForeground(QColor("#9ece6a"));
    } else {
        statusItem->setText("Error");
        statusItem->setForeground(QColor("#f7768e"));
    }
}

void MyForm::updateGlobalStats() {
    double totalSpeed = 0;
    for(auto t : tasks) {
        totalSpeed += t->currentSpeed;
    }
    lblGlobalSpeed->setText(formatSize(totalSpeed) + "/s");
    globalGraph->addPoint(totalSpeed);
}

void MyForm::onPauseClicked() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if(selected.isEmpty()) return;
    int row = selected[0]->row();
    for(auto t : tasks) {
        if(t->tableRow == row) {
            QMetaObject::invokeMethod(t->worker, "pauseDownload");
            break;
        }
    }
}

void MyForm::onResumeClicked() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if(selected.isEmpty()) return;
    int row = selected[0]->row();
    for(auto key : tasks.keys()) {
        if(tasks[key]->tableRow == row) {
            QString id = tasks[key]->downloadId;
            QMetaObject::invokeMethod(tasks[key]->worker, "resumeDownload", Q_ARG(QString, id)); 
            break;
        }
    }
}

void MyForm::onRemoveClicked() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if(selected.isEmpty()) return;
    int row = selected[0]->row();
    
    QString idToRemove;
    for(auto key : tasks.keys()) {
        if(tasks[key]->tableRow == row) {
            idToRemove = key;
            QMetaObject::invokeMethod(tasks[key]->worker, "cancelDownload");
            tasks[key]->thread->quit();
            tasks[key]->thread->wait();
            delete tasks[key]; 
            break;
        }
    }
    
    if(!idToRemove.isEmpty()) {
        tasks.remove(idToRemove);
        table->removeRow(row);
        for(auto t : tasks) {
            if(t->tableRow > row) t->tableRow--;
        }
    }
}

QString MyForm::formatSize(double bytes) {
    if (bytes < 0) return "0 B";
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    while (bytes >= 1024 && unit < 4) { bytes /= 1024; unit++; }
    return QString("%1 %2").arg(bytes, 0, 'f', 2).arg(units[unit]);
}

QString MyForm::formatTime(double seconds) {
    if (std::isinf(seconds) || std::isnan(seconds)) return "--";
    if (seconds <= 0) return "0s";
    if (seconds < 60) return QString("%1s").arg((int)seconds);
    if (seconds < 3600) return QString("%1m %2s").arg((int)seconds/60).arg((int)seconds%60);
    return QString("%1h %2m").arg((int)seconds/3600).arg(((int)seconds%3600)/60);
}