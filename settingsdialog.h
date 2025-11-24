#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    // Getters for settings
    QString getDefaultDownloadPath() const;
    int getDefaultConnections() const;
    bool getAutoStartDownloads() const;
    bool getNotificationsEnabled() const;
    bool getClipboardMonitoring() const;
    bool getShowSystemTrayIcon() const;
    double getDefaultSpeedLimit() const; // 0 = unlimited
    
    void loadSettings();
    void saveSettings();

private slots:
    void onBrowseClicked();
    void onAccepted();
    void onRejected();

private:
    void setupUI();
    void setupGeneralTab(QWidget* tab);
    void setupDownloadTab(QWidget* tab);
    void setupNotificationTab(QWidget* tab);
    
    QTabWidget* m_tabWidget;
    
    // General settings
    QLineEdit* m_defaultPathEdit;
    QCheckBox* m_showTrayIcon;
    
    // Download settings
    QSpinBox* m_defaultConnections;
    QCheckBox* m_autoStartDownloads;
    QCheckBox* m_clipboardMonitoring;
    QComboBox* m_speedLimitCombo;
    QSpinBox* m_customSpeedLimit;
    
    // Notification settings
    QCheckBox* m_enableNotifications;
    QCheckBox* m_notifyOnComplete;
    QCheckBox* m_notifyOnError;
    
    QSettings* m_settings;
};

#endif
