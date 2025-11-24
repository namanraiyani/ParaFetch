#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QString>

class NotificationManager : public QObject {
    Q_OBJECT
public:
    static NotificationManager& instance();
    
    void showNotification(const QString& title, const QString& message, 
                         const QString& icon = "");
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    NotificationManager();
    ~NotificationManager() = default;
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;
    
    void sendDbusNotification(const QString& title, const QString& message);
    
    bool m_enabled;
};

#endif
