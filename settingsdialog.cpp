#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    m_settings = new QSettings("ParaFetch", "ParaFetch", this);
    setupUI();
    loadSettings();
    
    setWindowTitle("Settings");
    resize(600, 450);
    
    // Apply dark theme styling
    resize(600, 450);
}

void SettingsDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget();
    
    // Create tabs
    QWidget* generalTab = new QWidget();
    QWidget* downloadTab = new QWidget();
    QWidget* notificationTab = new QWidget();
    
    setupGeneralTab(generalTab);
    setupDownloadTab(downloadTab);
    setupNotificationTab(notificationTab);
    
    m_tabWidget->addTab(generalTab, "General");
    m_tabWidget->addTab(downloadTab, "Downloads");
    m_tabWidget->addTab(notificationTab, "Notifications");
    
    mainLayout->addWidget(m_tabWidget);
    
    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply
    );
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, 
            this, &SettingsDialog::saveSettings);
    
    mainLayout->addWidget(buttons);
}

void SettingsDialog::setupGeneralTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGroupBox* pathGroup = new QGroupBox("Default Download Location");
    QHBoxLayout* pathLayout = new QHBoxLayout(pathGroup);
    
    m_defaultPathEdit = new QLineEdit();
    QPushButton* btnBrowse = new QPushButton("Browse...");
    btnBrowse->setMaximumWidth(100);
    connect(btnBrowse, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    
    pathLayout->addWidget(m_defaultPathEdit);
    pathLayout->addWidget(btnBrowse);
    
    layout->addWidget(pathGroup);
    
    QGroupBox* uiGroup = new QGroupBox("User Interface");
    QVBoxLayout* uiLayout = new QVBoxLayout(uiGroup);
    
    m_showTrayIcon = new QCheckBox("Show system tray icon");
    uiLayout->addWidget(m_showTrayIcon);
    
    layout->addWidget(uiGroup);
    layout->addStretch();
}

void SettingsDialog::setupDownloadTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGroupBox* connGroup = new QGroupBox("Connection Settings");
    QFormLayout* connLayout = new QFormLayout(connGroup);
    
    m_defaultConnections = new QSpinBox();
    m_defaultConnections->setRange(1, 32);
    m_defaultConnections->setValue(8);
    m_defaultConnections->setSuffix(" connections");
    connLayout->addRow("Default connections per download:", m_defaultConnections);
    
    layout->addWidget(connGroup);
    
    QGroupBox* speedGroup = new QGroupBox("Speed Limit");
    QFormLayout* speedLayout = new QFormLayout(speedGroup);
    
    m_speedLimitCombo = new QComboBox();
    m_speedLimitCombo->addItems({"Unlimited", "512 KB/s", "1 MB/s", "5 MB/s", "10 MB/s", "Custom"});
    
    m_customSpeedLimit = new QSpinBox();
    m_customSpeedLimit->setRange(1, 99999);
    m_customSpeedLimit->setValue(1024);
    m_customSpeedLimit->setSuffix(" KB/s");
    m_customSpeedLimit->setEnabled(false);
    
    connect(m_speedLimitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
        m_customSpeedLimit->setEnabled(index == 5); // Enable for "Custom"
    });
    
    speedLayout->addRow("Default speed limit:", m_speedLimitCombo);
    speedLayout->addRow("Custom limit:", m_customSpeedLimit);
    
    layout->addWidget(speedGroup);
    
    QGroupBox* behaviorGroup = new QGroupBox("Behavior");
    QVBoxLayout* behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    m_autoStartDownloads = new QCheckBox("Automatically start downloads when added");
    m_autoStartDownloads->setChecked(true);
    behaviorLayout->addWidget(m_autoStartDownloads);
    
    m_clipboardMonitoring = new QCheckBox("Monitor clipboard for download URLs");
    behaviorLayout->addWidget(m_clipboardMonitoring);
    
    layout->addWidget(behaviorGroup);
    layout->addStretch();
}

void SettingsDialog::setupNotificationTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGroupBox* notifGroup = new QGroupBox("Desktop Notifications");
    QVBoxLayout* notifLayout = new QVBoxLayout(notifGroup);
    
    m_enableNotifications = new QCheckBox("Enable desktop notifications");
    m_enableNotifications->setChecked(true);
    notifLayout->addWidget(m_enableNotifications);
    
    m_notifyOnComplete = new QCheckBox("Notify when download completes");
    m_notifyOnComplete->setChecked(true);
    notifLayout->addWidget(m_notifyOnComplete);
    
    m_notifyOnError = new QCheckBox("Notify when download fails");
    m_notifyOnError->setChecked(true);
    notifLayout->addWidget(m_notifyOnError);
    
    layout->addWidget(notifGroup);
    layout->addStretch();
}

void SettingsDialog::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this, 
        "Select Default Download Directory", 
        m_defaultPathEdit->text()
    );
    if (!dir.isEmpty()) {
        m_defaultPathEdit->setText(dir);
    }
}

void SettingsDialog::onAccepted() {
    saveSettings();
    accept();
}

void SettingsDialog::onRejected() {
    reject();
}

void SettingsDialog::loadSettings() {
    m_defaultPathEdit->setText(
        m_settings->value("DefaultDownloadPath", QDir::homePath() + "/Downloads").toString()
    );
    m_defaultConnections->setValue(
        m_settings->value("DefaultConnections", 8).toInt()
    );
    m_autoStartDownloads->setChecked(
        m_settings->value("AutoStartDownloads", true).toBool()
    );
    m_clipboardMonitoring->setChecked(
        m_settings->value("ClipboardMonitoring", false).toBool()
    );
    m_showTrayIcon->setChecked(
        m_settings->value("ShowTrayIcon", false).toBool()
    );
    
    // Speed limit
    double speedLimit = m_settings->value("DefaultSpeedLimit", 0.0).toDouble();
    if (speedLimit == 0) {
        m_speedLimitCombo->setCurrentIndex(0); // Unlimited
    } else if (speedLimit == 512) {
        m_speedLimitCombo->setCurrentIndex(1);
    } else if (speedLimit == 1024) {
        m_speedLimitCombo->setCurrentIndex(2);
    } else if (speedLimit == 5120) {
        m_speedLimitCombo->setCurrentIndex(3);
    } else if (speedLimit == 10240) {
        m_speedLimitCombo->setCurrentIndex(4);
    } else {
        m_speedLimitCombo->setCurrentIndex(5); // Custom
        m_customSpeedLimit->setValue((int)speedLimit);
    }
    
    // Notifications
    m_enableNotifications->setChecked(
        m_settings->value("NotificationsEnabled", true).toBool()
    );
    m_notifyOnComplete->setChecked(
        m_settings->value("NotifyOnComplete", true).toBool()
    );
    m_notifyOnError->setChecked(
        m_settings->value("NotifyOnError", true).toBool()
    );
}

void SettingsDialog::saveSettings() {
    m_settings->setValue("DefaultDownloadPath", m_defaultPathEdit->text());
    m_settings->setValue("DefaultConnections", m_defaultConnections->value());
    m_settings->setValue("AutoStartDownloads", m_autoStartDownloads->isChecked());
    m_settings->setValue("ClipboardMonitoring", m_clipboardMonitoring->isChecked());
    m_settings->setValue("ShowTrayIcon", m_showTrayIcon->isChecked());
    m_settings->setValue("NotificationsEnabled", m_enableNotifications->isChecked());
    m_settings->setValue("NotifyOnComplete", m_notifyOnComplete->isChecked());
    m_settings->setValue("NotifyOnError", m_notifyOnError->isChecked());
    
    // Speed limit
    double speedLimit = 0.0;
    int idx = m_speedLimitCombo->currentIndex();
    if (idx == 1) speedLimit = 512;
    else if (idx == 2) speedLimit = 1024;
    else if (idx == 3) speedLimit = 5120;
    else if (idx == 4) speedLimit = 10240;
    else if (idx == 5) speedLimit = m_customSpeedLimit->value();
    
    m_settings->setValue("DefaultSpeedLimit", speedLimit);
    m_settings->sync();
}

// Getters
QString SettingsDialog::getDefaultDownloadPath() const {
    return m_settings->value("DefaultDownloadPath", QDir::homePath() + "/Downloads").toString();
}

int SettingsDialog::getDefaultConnections() const {
    return m_settings->value("DefaultConnections", 8).toInt();
}

bool SettingsDialog::getAutoStartDownloads() const {
    return m_settings->value("AutoStartDownloads", true).toBool();
}

bool SettingsDialog::getNotificationsEnabled() const {
    return m_settings->value("NotificationsEnabled", true).toBool();
}

bool SettingsDialog::getClipboardMonitoring() const {
    return m_settings->value("ClipboardMonitoring", false).toBool();
}

bool SettingsDialog::getShowSystemTrayIcon() const {
    return m_settings->value("ShowTrayIcon", false).toBool();
}

double SettingsDialog::getDefaultSpeedLimit() const {
    return m_settings->value("DefaultSpeedLimit", 0.0).toDouble();
}
