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
#include <QToolButton>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDesktopServices>
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
}

// --- Main Form Constructor ---
MyForm::MyForm(QWidget *parent) : QMainWindow(parent) {
    // Initialize settings
    settings = new QSettings("ParaFetch", "ParaFetch", this);
    loadSettings();
    
    // Setup UI components
    setupUI();
    applyStyles();
    
    // Enable drag and drop
    setAcceptDrops(true);
    
    // Setup clipboard monitoring
    clipboard = QApplication::clipboard();
    if (clipboardMonitoringEnabled) {
        connect(clipboard, &QClipboard::dataChanged, this, &MyForm::onClipboardChanged);
    }
    
    // Setup system tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon::fromTheme("download"));
    trayIcon->setToolTip("ParaFetch Download Manager");
    
    QMenu* trayMenu = new QMenu(this);
    trayMenu->addAction("Show", this, &QWidget::show);
    trayMenu->addAction("Hide", this, &QWidget::hide);
    trayMenu->addSeparator();
    trayMenu->addAction("Quit", qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MyForm::onTrayIconActivated);
    
    if (settings->value("ShowTrayIcon", false).toBool()) {
        trayIcon->show();
    }

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
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    
    QAction* actAdd = toolbar->addAction("+");
    QAction* actBatchAdd = toolbar->addAction("Batch Download");
    actPauseResume = toolbar->addAction("⏸");
    QAction* actRemove = toolbar->addAction("Remove");

    // Apply specific styles to buttons
    if (QWidget* btn = toolbar->widgetForAction(actAdd)) {
        btn->setFixedSize(40, 40);
        btn->setStyleSheet("font-size: 20px;");
    }
    if (QWidget* btn = toolbar->widgetForAction(actBatchAdd)) {
        btn->setFixedSize(140, 40);
    }
    if (QWidget* btn = toolbar->widgetForAction(actPauseResume)) {
        btn->setFixedSize(40, 40);
        btn->setStyleSheet("font-size: 18px; padding-bottom: 2px;");
    }
    if (QWidget* btn = toolbar->widgetForAction(actRemove)) {
        btn->setFixedSize(90, 40);
    }

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);
    
    QAction* actSettings = toolbar->addAction("⚙");
    if (QWidget* btn = toolbar->widgetForAction(actSettings)) {
        btn->setFixedSize(40, 40);
        btn->setStyleSheet("font-size: 18px;"); 
    }

    connect(actAdd, &QAction::triggered, this, &MyForm::onAddClicked);
    connect(actBatchAdd, &QAction::triggered, this, &MyForm::onBatchAddClicked);
    connect(actPauseResume, &QAction::triggered, this, &MyForm::onPauseResumeToggle);
    connect(actRemove, &QAction::triggered, this, &MyForm::onRemoveClicked);
    connect(actSettings, &QAction::triggered, this, &MyForm::onSettingsClicked);

    // Table
    table = new QTableWidget();
    table->setColumnCount(7); 
    table->setHorizontalHeaderLabels({"Name", "Downloaded", "Size", "Progress", "Speed", "ETA", "Status"});
    
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    
    // Set default row height to be larger than the progress bar height
    table->verticalHeader()->setDefaultSectionSize(45); // INCREASED SIZE
    
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
    table->setAlternatingRowColors(false);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table, &QTableWidget::itemSelectionChanged, this, &MyForm::updatePauseResumeButton);
    connect(table, &QTableWidget::customContextMenuRequested, this, &MyForm::showContextMenu);
    mainLayout->addWidget(table);

    // Bottom Panel
    QFrame* bottomPanel = new QFrame();
    bottomPanel->setFixedHeight(120);
    bottomPanel->setObjectName("BottomPanel");
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomPanel);
    
    globalGraph = new GlobalSpeedGraph();
    DiskUsagePieChart* diskChart = new DiskUsagePieChart();
    
    lblGlobalSpeed = new QLabel("0.00 MB/s");
    lblGlobalSpeed->setStyleSheet(R"(
        font-size: 28px; 
        font-weight: 600; 
        color: #FFFFFF;
    )");

    QLabel* titleLabel = new QLabel("TOTAL DOWNLOAD SPEED");
    titleLabel->setStyleSheet("color: #8E8E93; font-size: 11px; font-weight: 600; letter-spacing: 0.5px;");

    QVBoxLayout* statsLayout = new QVBoxLayout();
    statsLayout->addWidget(titleLabel);
    statsLayout->addWidget(lblGlobalSpeed);
    statsLayout->addStretch();

    bottomLayout->addLayout(statsLayout);
    bottomLayout->addWidget(globalGraph, 1); 
    bottomLayout->addWidget(diskChart);

    mainLayout->addWidget(bottomPanel);
}

void MyForm::applyStyles() {
    qApp->setStyleSheet(R"(
        /* Main Window & Dialogs - MacOS Dark Mode Surface */
        QMainWindow, QDialog { 
            background-color: #1C1C1E;
            color: #F2F2F7;
        }

        /* Input Fields */
        QLineEdit {
            background-color: #2C2C2E;
            color: #FFFFFF;
            border: 1px solid #3A3A3C;
            border-radius: 6px;
            padding: 6px;
            selection-background-color: #0A84FF;
        }
        
        QLineEdit:focus {
            border: 1px solid #0A84FF;
        }

        /* Push Buttons (Dialogs) */
        QPushButton {
            background-color: #3A3A3C;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: 600;
        }
        
        QPushButton:hover {
            background-color: #48484A;
        }
        
        QPushButton:pressed {
            background-color: #2C2C2E;
        }
        
        QPushButton:default {
            background-color: #0A84FF;
        }
        
        QPushButton:default:hover {
            background-color: #007AFF;
        }

        /* Tab Widget */
        QTabWidget::pane {
            border: 1px solid #3A3A3C;
            background-color: #1C1C1E;
        }
        
        QTabBar::tab {
            background: #2C2C2E;
            color: #8E8E93;
            padding: 8px 16px;
            border: none;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background: #3A3A3C;
            color: #FFFFFF;
            font-weight: 600;
        }

        /* Group Box */
        QGroupBox {
            border: 1px solid #3A3A3C;
            border-radius: 6px;
            margin-top: 20px;
            padding-top: 10px;
            color: #8E8E93;
            font-weight: 600;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            left: 10px;
        }

        /* Check Box */
        QCheckBox {
            color: #E5E5EA;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid #48484A;
            border-radius: 4px;
            background-color: #2C2C2E;
        }
        
        QCheckBox::indicator:checked {
            background-color: #0A84FF;
            border-color: #0A84FF;
            image: url(:/icons/check.png); /* Assuming we might have an icon, or just color */
        }

        /* ComboBox & SpinBox */
        QComboBox, QSpinBox {
            background-color: #2C2C2E;
            color: #FFFFFF;
            border: 1px solid #3A3A3C;
            border-radius: 6px;
            padding: 6px;
        }
        
        QComboBox::drop-down {
            border: none;
        }

        /* Toolbar - Flat, clean, subtle border */
        QToolBar { 
            background-color: #2C2C2E;
            border: none;
            border-bottom: 1px solid #3A3A3C;
            padding: 10px;
            spacing: 15px;
        }
        
        /* Toolbar Buttons - Rounded, subtle hover */
        QToolButton { 
            background-color: transparent;
            color: #FFFFFF;
            font-weight: 600;
            font-size: 14px;
            padding: 8px 12px;
            border: none;
            border-radius: 6px;
        }
        
        QToolButton:hover { 
            background-color: rgba(255, 255, 255, 0.1);
        }
        
        QToolButton:pressed {
            background-color: rgba(255, 255, 255, 0.15);
        }
        
        QToolButton:disabled {
            color: #636366;
        }
        
        /* Table Widget - Clean, no grid, alternating rows */
        QTableWidget { 
            background-color: #1C1C1E;
            color: #F2F2F7;
            border: none;
            gridline-color: transparent;
            font-size: 13px;
            selection-background-color: #0A84FF;
            selection-color: #FFFFFF;
            outline: none;
        }
        
        QTableWidget::item { 
            padding: 5px 10px;
            border: none;
            border-bottom: 1px solid #2C2C2E;
        }
        
        QTableWidget::item:selected { 
            background-color: #0A84FF;
            color: #FFFFFF;
        }
        
        /* Table Header - Flat, bold */
        QHeaderView::section { 
            background-color: #1C1C1E;
            color: #8E8E93;
            padding: 8px 10px;
            border: none;
            border-bottom: 1px solid #3A3A3C;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
        }
        
        /* Scrollbar - Minimalist */
        QScrollBar:vertical {
            background: #1C1C1E;
            width: 12px;
            margin: 0px;
        }
        
        QScrollBar::handle:vertical {
            background: #48484A;
            min-height: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: #636366;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QScrollBar:horizontal {
            background: #1C1C1E;
            height: 12px;
            margin: 0px;
        }
        
        QScrollBar::handle:horizontal {
            background: #48484A;
            min-width: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background: #636366;
        }
        
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        
        /* Bottom Panel */
        QFrame#BottomPanel { 
            background-color: #2C2C2E;
            border-top: 1px solid #3A3A3C;
        }
        
        QLabel { 
            color: #E5E5EA;
            font-size: 13px;
        }
    )");
}

// --- Actions ---

void MyForm::loadSettings() {
    defaultDownloadPath = settings->value("DefaultDownloadPath", QDir::homePath() + "/Downloads").toString();
    defaultConnections = settings->value("DefaultConnections", 8).toInt();
    defaultSpeedLimit = settings->value("DefaultSpeedLimit", 0.0).toDouble();
    clipboardMonitoringEnabled = settings->value("ClipboardMonitoring", false).toBool();
    notificationsEnabled = settings->value("NotificationsEnabled", true).toBool();
    NotificationManager::instance().setEnabled(notificationsEnabled);
}

void MyForm::onSettingsClicked() {
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        settings->sync(); // Force reload from disk/memory
        loadSettings();
        if (settings->value("ShowTrayIcon", false).toBool()) {
            trayIcon->show();
        } else {
            trayIcon->hide();
        }
        
        if (clipboardMonitoringEnabled) {
             connect(clipboard, &QClipboard::dataChanged, this, &MyForm::onClipboardChanged, Qt::UniqueConnection);
        } else {
             disconnect(clipboard, &QClipboard::dataChanged, this, &MyForm::onClipboardChanged);
        }
    }
}

void MyForm::onBatchAddClicked() {
    BatchDownloadDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList urls = dlg.getUrls();
        QString path = dlg.getSavePath();
        
        for (const QString& url : urls) {
            addDownload(url, path);
        }
    }
}

void MyForm::addDownload(const QString& url, const QString& path) {
    if(url.isEmpty()) return;

    TaskInfo* task = new TaskInfo();
    task->thread = new QThread();
    task->worker = new DownloadWorker();
    task->worker->moveToThread(task->thread);
    task->currentSpeed = 0;
    task->url = url;
    task->outputPath = path;

    int row = table->rowCount();
    table->insertRow(row);
    
    QString fileName = QFileInfo(QUrl(url).path()).fileName();
    if(fileName.isEmpty()) fileName = "downloading...";
    table->setItem(row, 0, new QTableWidgetItem(fileName)); 
    
    table->setItem(row, 1, new QTableWidgetItem("0 B"));
    table->setItem(row, 2, new QTableWidgetItem("--"));
    
    QWidget* progressContainer = new QWidget();
    QVBoxLayout* progressLayout = new QVBoxLayout(progressContainer);
    progressLayout->setContentsMargins(10, 5, 10, 5); 
    progressLayout->setSpacing(0);
    
    TableSegmentedBar* pBar = new TableSegmentedBar();
    progressLayout->addWidget(pBar);
    
    table->setCellWidget(row, 3, progressContainer);

    table->setItem(row, 4, new QTableWidgetItem("0 B/s"));
    table->setItem(row, 5, new QTableWidgetItem("--"));
    
    QTableWidgetItem* statusItem = new QTableWidgetItem("Downloading");
    statusItem->setForeground(QColor("#e0af68"));
    table->setItem(row, 6, statusItem);
    
    task->tableRow = row;
    QString uid = QString::number((quintptr)task); 
    tasks.insert(uid, task);

    connect(task->thread, &QThread::started, task->worker, [=](){ 
        if (defaultSpeedLimit > 0) {
             QMetaObject::invokeMethod(task->worker, "setSpeedLimit", Q_ARG(double, defaultSpeedLimit));
        }
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

void MyForm::onAddClicked() {
    AddDownloadDialog dlg(this);
    // Set default path from settings
    dlg.pathEdit->setText(defaultDownloadPath);
    
    if(dlg.exec() == QDialog::Accepted) {
        QString url = dlg.urlEdit->text().trimmed();
        QString path = dlg.pathEdit->text();
        addDownload(url, path);
    }
}

// Drag & Drop
void MyForm::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void MyForm::dropEvent(QDropEvent *event) {
    QString url;
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) url = urls.first().toString();
    } else if (event->mimeData()->hasText()) {
        url = event->mimeData()->text();
    }
    
    if (!url.isEmpty()) {
        if (url.startsWith("http") || url.startsWith("ftp")) {
            AddDownloadDialog dlg(this);
            dlg.urlEdit->setText(url);
            dlg.pathEdit->setText(defaultDownloadPath);
            if (dlg.exec() == QDialog::Accepted) {
                addDownload(dlg.urlEdit->text(), dlg.pathEdit->text());
            }
        }
    }
    event->acceptProposedAction();
}

// Clipboard
void MyForm::onClipboardChanged() {
    if (!clipboardMonitoringEnabled) return;
    
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasText()) {
        QString text = mimeData->text().trimmed();
        if ((text.startsWith("http://") || text.startsWith("https://")) && 
            !text.contains(" ")) {
            // Optional: Show notification or small popup
            // For now, we'll just log it or maybe flash the tray icon
            // Implementing full auto-add might be annoying without a specific "Monitor" mode toggle
        }
    }
}

// Tray Icon
void MyForm::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) hide();
        else {
            show();
            activateWindow();
        }
    }
}

// Context Menu
void MyForm::showContextMenu(const QPoint &pos) {
    QTableWidgetItem *item = table->itemAt(pos);
    if (!item) return;
    
    int row = item->row();
    table->selectRow(row);
    
    QMenu menu(this);
    QAction* openFolderAct = menu.addAction("Open Folder");
    QAction* openFileAct = menu.addAction("Open File");
    QAction* copyUrlAct = menu.addAction("Copy URL");
    menu.addSeparator();
    QAction* removeAct = menu.addAction("Remove");
    
    QTableWidgetItem* statusItem = table->item(row, 6);
    bool isCompleted = (statusItem->text() == "Completed");
    openFileAct->setEnabled(isCompleted);
    
    QAction* selectedItem = menu.exec(table->viewport()->mapToGlobal(pos));
    if (selectedItem == openFolderAct) {
        openDownloadFolder(row);
    } else if (selectedItem == openFileAct) {
        openDownloadedFile(row);
    } else if (selectedItem == copyUrlAct) {
        copyUrlToClipboard(row);
    } else if (selectedItem == removeAct) {
        onRemoveClicked();
    }
}

void MyForm::openDownloadFolder(int row) {
    for(auto t : tasks) {
        if(t->tableRow == row) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(t->outputPath));
            return;
        }
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(defaultDownloadPath));
}

void MyForm::openDownloadedFile(int row) {
    for(auto t : tasks) {
        if(t->tableRow == row) {
            QString fileName = table->item(row, 0)->text();
            QString fullPath = t->outputPath + "/" + fileName;
            QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
            return;
        }
    }
}

void MyForm::copyUrlToClipboard(int row) {
    for(auto t : tasks) {
        if(t->tableRow == row) {
            QApplication::clipboard()->setText(t->url);
            return;
        }
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

    QWidget* container = table->cellWidget(row, 3);
    TableSegmentedBar* pBar = container ? container->findChild<TableSegmentedBar*>() : nullptr;
    if(pBar) pBar->setProgressData(dl, total); 

    table->item(row, 1)->setText(formatSize(dl));
    table->item(row, 2)->setText(formatSize(total));
    table->item(row, 4)->setText(formatSize(speed) + "/s");
    table->item(row, 5)->setText(formatTime(eta));
}

void MyForm::onWorkerChunkProgress(QString id, const std::vector<ChunkProgress>& chunks) {
    if(!tasks.contains(id)) return;
    int row = tasks[id]->tableRow;
    
    QWidget* container = table->cellWidget(row, 3);
    TableSegmentedBar* pBar = container ? container->findChild<TableSegmentedBar*>() : nullptr;
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
    
    // Update pause/resume button text if this task is currently selected
    updatePauseResumeButton();
}

void MyForm::onWorkerFinished(QString id, bool success, QString msg) {
    if(!tasks.contains(id)) return;
    TaskInfo* t = tasks[id];
    t->currentSpeed = 0; 
    
    QTableWidgetItem* statusItem = table->item(t->tableRow, 6);
    
    if (success) {
        statusItem->setText("Completed");
        statusItem->setForeground(QColor("#9ece6a"));
        
        if (settings->value("NotifyOnComplete", true).toBool()) {
            QString fileName = table->item(t->tableRow, 0)->text();
            NotificationManager::instance().showNotification(
                "Download Complete", 
                fileName + " has finished downloading."
            );
        }
    } else {
        statusItem->setText("Error");
        statusItem->setForeground(QColor("#f7768e"));
        
        if (settings->value("NotifyOnError", true).toBool()) {
            QString fileName = table->item(t->tableRow, 0)->text();
            NotificationManager::instance().showNotification(
                "Download Failed", 
                fileName + " failed: " + msg
            );
        }
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

void MyForm::onPauseResumeToggle() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if(selected.isEmpty()) return;
    int row = selected[0]->row();
    
    // Get the status of the selected task
    QTableWidgetItem* statusItem = table->item(row, 6);
    QString status = statusItem ? statusItem->text() : "";
    
    for(auto key : tasks.keys()) {
        if(tasks[key]->tableRow == row) {
            if(status == "Paused") {
                // Resume the download
                QString id = tasks[key]->downloadId;
                QMetaObject::invokeMethod(tasks[key]->worker, "resumeDownload", Q_ARG(QString, id));
            } else {
                // Pause the download
                QMetaObject::invokeMethod(tasks[key]->worker, "pauseDownload");
            }
            break;
        }
    }
}

void MyForm::updatePauseResumeButton() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if(selected.isEmpty()) {
        actPauseResume->setText("⏸");
        actPauseResume->setEnabled(false);
        return;
    }
    
    int row = selected[0]->row();
    QTableWidgetItem* statusItem = table->item(row, 6);
    QString status = statusItem ? statusItem->text() : "";
    
    actPauseResume->setEnabled(true);
    
    if(status == "Paused") {
        actPauseResume->setText("▶");
    } else if(status == "Downloading") {
        actPauseResume->setText("⏸");
    } else {
        // For completed/error states, disable the button
        actPauseResume->setEnabled(false);
        actPauseResume->setText("⏸");
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