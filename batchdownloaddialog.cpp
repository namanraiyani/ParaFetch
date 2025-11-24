#include "batchdownloaddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QDir>

BatchDownloadDialog::BatchDownloadDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
    setWindowTitle("Batch Download - Import URLs");
    resize(700, 500);
    
    resize(700, 500);
}

void BatchDownloadDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Instructions
    QLabel* instructions = new QLabel(
        "Enter URLs (one per line) or load from a text file:"
    );
    instructions->setStyleSheet("color: #7aa2f7; font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(instructions);
    
    // URL text area with buttons
    QHBoxLayout* textAreaLayout = new QHBoxLayout();
    
    m_urlTextEdit = new QTextEdit();
    m_urlTextEdit->setPlaceholderText("https://example.com/file1.zip\nhttps://example.com/file2.iso\n...");
    textAreaLayout->addWidget(m_urlTextEdit, 1);
    
    QVBoxLayout* textBtnLayout = new QVBoxLayout();
    m_loadFileBtn = new QPushButton("Load from\nFile...");
    m_validateBtn = new QPushButton("Validate\nURLs");
    textBtnLayout->addWidget(m_loadFileBtn);
    textBtnLayout->addWidget(m_validateBtn);
    textBtnLayout->addStretch();
    textAreaLayout->addLayout(textBtnLayout);
    
    mainLayout->addLayout(textAreaLayout);
    
    // Preview list
    QLabel* previewLabel = new QLabel("Valid URLs (0):");
    previewLabel->setStyleSheet("color: #7aa2f7; font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(previewLabel);
    
    m_urlList = new QListWidget();
    m_urlList->setMaximumHeight(120);
    mainLayout->addWidget(m_urlList);
    
    // Save path
    QLabel* pathLabel = new QLabel("Save all files to:");
    pathLabel->setStyleSheet("color: #7aa2f7; font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(pathLabel);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(QDir::homePath() + "/Downloads");
    QPushButton* btnBrowse = new QPushButton("Browse...");
    btnBrowse->setMaximumWidth(100);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(btnBrowse);
    mainLayout->addLayout(pathLayout);
    
    // Dialog buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    mainLayout->addWidget(buttons);
    
    // Connections
    connect(m_loadFileBtn, &QPushButton::clicked, this, &BatchDownloadDialog::onLoadFromFile);
    connect(btnBrowse, &QPushButton::clicked, this, &BatchDownloadDialog::onBrowsePath);
    connect(m_validateBtn, &QPushButton::clicked, this, &BatchDownloadDialog::onValidateUrls);
    connect(buttons, &QDialogButtonBox::accepted, this, &BatchDownloadDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BatchDownloadDialog::onLoadFromFile() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Select URL List File",
        QDir::homePath(),
        "Text Files (*.txt);;All Files (*)"
    );
    
    if (filename.isEmpty()) return;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + filename);
        return;
    }
    
    QTextStream in(&file);
    QString content;
    while (!in.atEnd()) {
        content += in.readLine() + "\n";
    }
    file.close();
    
    m_urlTextEdit->setPlainText(content);
    onValidateUrls(); // Auto-validate after loading
}

void BatchDownloadDialog::onBrowsePath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Save Directory",
        m_pathEdit->text()
    );
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void BatchDownloadDialog::onValidateUrls() {
    m_urlList->clear();
    m_urls.clear();
    
    QString text = m_urlTextEdit->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('#')) continue; // Skip empty and comments
        
        if (isValidUrl(trimmed)) {
            m_urls.append(trimmed);
            m_urlList->addItem(trimmed);
        }
    }
    
    // Update label
    QLabel* previewLabel = qobject_cast<QLabel*>(
        m_urlList->parentWidget()->layout()->itemAt(
            m_urlList->parentWidget()->layout()->indexOf(m_urlList) - 1
        )->widget()
    );
    if (previewLabel) {
        previewLabel->setText(QString("Valid URLs (%1):").arg(m_urls.count()));
    }
}

void BatchDownloadDialog::onAccepted() {
    // Validate before accepting
    onValidateUrls();
    
    if (m_urls.isEmpty()) {
        QMessageBox::warning(this, "No URLs", "Please enter at least one valid URL.");
        return;
    }
    
    if (m_pathEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "No Path", "Please specify a save directory.");
        return;
    }
    
    accept();
}

bool BatchDownloadDialog::isValidUrl(const QString& url) const {
    QUrl qurl(url);
    return qurl.isValid() && 
           (qurl.scheme() == "http" || qurl.scheme() == "https" || qurl.scheme() == "ftp");
}

QString BatchDownloadDialog::getSavePath() const {
    return m_pathEdit->text();
}
