#ifndef BATCHDOWNLOADDIALOG_H
#define BATCHDOWNLOADDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class BatchDownloadDialog : public QDialog {
    Q_OBJECT
public:
    explicit BatchDownloadDialog(QWidget *parent = nullptr);
    
    QStringList getUrls() const { return m_urls; }
    QString getSavePath() const;

private slots:
    void onLoadFromFile();
    void onBrowsePath();
    void onValidateUrls();
    void onAccepted();

private:
    void setupUI();
    bool isValidUrl(const QString& url) const;
    
    QTextEdit* m_urlTextEdit;
    QLineEdit* m_pathEdit;
    QListWidget* m_urlList;
    QPushButton* m_loadFileBtn;
    QPushButton* m_validateBtn;
    QStringList m_urls;
};

#endif
