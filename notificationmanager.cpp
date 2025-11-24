#include "notificationmanager.h"
#include <QProcess>
#include <QDebug>

NotificationManager& NotificationManager::instance() {
    static NotificationManager instance;
    return instance;
}

NotificationManager::NotificationManager() : m_enabled(true) {
}

void NotificationManager::showNotification(const QString& title, 
                                          const QString& message,
                                          const QString& icon) {
    if (!m_enabled) return;
    
    // Try to use system notifications via notify-send (Linux)
    // This is a simple cross-platform approach
    sendDbusNotification(title, message);
}

void NotificationManager::sendDbusNotification(const QString& title, 
                                               const QString& message) {
    // Use notify-send command (available on most Linux desktop environments)
    QProcess process;
    QStringList args;
    args << "-a" << "ParaFetch"
         << "-i" << "download" // Generic download icon
         << title
         << message;
    
    process.startDetached("notify-send", args);
    
    // Fallback: just log if notify-send is not available
    qDebug() << "Notification:" << title << "-" << message;
}
