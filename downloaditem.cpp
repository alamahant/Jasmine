#include "downloaditem.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QTime>
#include <QStyle>

DownloadItem::DownloadItem(QWebEngineDownloadRequest *download, QWidget *parent)
    : QWidget(parent)
    , m_download(download)
    , m_lastReceivedBytes(0)
    , m_lastUpdateTime(QTime::currentTime())
{
    setupUI();

    // Connect download signals
    connect(m_download, &QWebEngineDownloadRequest::receivedBytesChanged,
            this, &DownloadItem::updateProgress);
    connect(m_download, &QWebEngineDownloadRequest::stateChanged,
            this, &DownloadItem::onStateChanged);

    // Initial update
    updateProgress();
    updateButtons();
}

void DownloadItem::setupUI()
{
    setFixedHeight(80);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 5);

    // Left side - file info and progress
    QVBoxLayout *infoLayout = new QVBoxLayout();

    // File name and size
    QHBoxLayout *fileInfoLayout = new QHBoxLayout();
    m_fileNameLabel = new QLabel(m_download->downloadFileName());
    m_fileNameLabel->setStyleSheet("font-weight: bold;");
    m_fileSizeLabel = new QLabel();

    fileInfoLayout->addWidget(m_fileNameLabel);
    fileInfoLayout->addStretch();
    fileInfoLayout->addWidget(m_fileSizeLabel);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);

    // Status label
    m_statusLabel = new QLabel("Starting download...");
    m_statusLabel->setStyleSheet("color: gray; font-size: 11px;");

    infoLayout->addLayout(fileInfoLayout);
    infoLayout->addWidget(m_progressBar);
    infoLayout->addWidget(m_statusLabel);

    mainLayout->addLayout(infoLayout, 1);

    // Right side - buttons
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    QHBoxLayout *topButtons = new QHBoxLayout();

    m_cancelButton = new QPushButton();
    m_cancelButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_cancelButton->setToolTip("Cancel");
    m_cancelButton->setFixedSize(30, 30);

    topButtons->addWidget(m_cancelButton);

    QHBoxLayout *bottomButtons = new QHBoxLayout();
    m_openFileButton = new QPushButton();
    m_openFileButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    m_openFileButton->setToolTip("Open File");
    m_openFileButton->setFixedSize(30, 30);
    m_openFileButton->setVisible(false);

    m_openFolderButton = new QPushButton();
    m_openFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_openFolderButton->setToolTip("Open Folder");
    m_openFolderButton->setFixedSize(30, 30);

    m_removeButton = new QPushButton();
    m_removeButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_removeButton->setToolTip("Remove from List");
    m_removeButton->setFixedSize(30, 30);
    m_removeButton->setVisible(false);

    bottomButtons->addWidget(m_openFileButton);
    bottomButtons->addWidget(m_openFolderButton);
    bottomButtons->addWidget(m_removeButton);

    buttonLayout->addLayout(topButtons);
    buttonLayout->addLayout(bottomButtons);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    m_cancelButton->setToolTip("Cancel Download");
    m_openFileButton->setToolTip("Open Downloaded File");
    m_openFolderButton->setToolTip("Open Download Folder");
    m_removeButton->setToolTip("Remove from List");
    // Connect button signals
    connect(m_cancelButton, &QPushButton::clicked, this, &DownloadItem::onCancelClicked);
    connect(m_openFileButton, &QPushButton::clicked, this, &DownloadItem::onOpenFileClicked);
    connect(m_openFolderButton, &QPushButton::clicked, this, &DownloadItem::onOpenFolderClicked);
    connect(m_removeButton, &QPushButton::clicked, [this]() {
        emit removeRequested(this);
    });
}

void DownloadItem::updateProgress()
{
    qint64 received = m_download->receivedBytes();
    qint64 total = m_download->totalBytes();

    // Update file size label
    if (total > 0) {
        m_fileSizeLabel->setText(QString("%1 / %2")
                                     .arg(formatFileSize(received))
                                     .arg(formatFileSize(total)));

        int progress = (received * 100) / total;
        m_progressBar->setValue(progress);
    } else {
        m_fileSizeLabel->setText(formatFileSize(received));
        m_progressBar->setRange(0, 0); // Indeterminate progress
    }

    // Calculate speed
    QTime currentTime = QTime::currentTime();
    int elapsed = m_lastUpdateTime.msecsTo(currentTime);

    if (elapsed > 1000) { // Update speed every second
        qint64 bytesDiff = received - m_lastReceivedBytes;
        qint64 speed = (bytesDiff * 1000) / elapsed;

        if (m_download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
            QString speedText = formatSpeed(speed);
            if (total > 0 && speed > 0) {
                int remainingSeconds = (total - received) / speed;
                speedText += " - " + formatTime(remainingSeconds) + " remaining";
            }
            m_statusLabel->setText(speedText);
        }

        m_lastReceivedBytes = received;
        m_lastUpdateTime = currentTime;
    }
}

void DownloadItem::onStateChanged()
{
    updateButtons();

    switch (m_download->state()) {
    case QWebEngineDownloadRequest::DownloadCompleted:
        m_statusLabel->setText("Download completed");
        m_progressBar->setValue(100);
        break;
    case QWebEngineDownloadRequest::DownloadCancelled:
        m_statusLabel->setText("Download cancelled");
        break;
    case QWebEngineDownloadRequest::DownloadInterrupted:
        m_statusLabel->setText("Download interrupted");
        break;
    default:
        break;
    }
}

void DownloadItem::updateButtons()
{
    bool isActive = (m_download->state() == QWebEngineDownloadRequest::DownloadInProgress);
    bool isFinished = (m_download->state() == QWebEngineDownloadRequest::DownloadCompleted);
    bool isCancelled = (m_download->state() == QWebEngineDownloadRequest::DownloadCancelled);



    m_cancelButton->setVisible(isActive);
    m_openFileButton->setVisible(isFinished);
    m_removeButton->setVisible(isCancelled);

}






void DownloadItem::onCancelClicked()
{
    m_download->cancel();
}

void DownloadItem::onOpenFileClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_download->downloadDirectory() + "/" + m_download->downloadFileName()));
}

void DownloadItem::onOpenFolderClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_download->downloadDirectory()));
}

bool DownloadItem::isFinished() const
{
    return m_download->state() == QWebEngineDownloadRequest::DownloadCompleted ||
           m_download->state() == QWebEngineDownloadRequest::DownloadCancelled ||
           m_download->state() == QWebEngineDownloadRequest::DownloadInterrupted;
}

QString DownloadItem::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

QString DownloadItem::formatSpeed(qint64 bytesPerSecond)
{
    return formatFileSize(bytesPerSecond) + "/s";
}

QString DownloadItem::formatTime(int seconds)
{
    if (seconds < 60) return QString("%1s").arg(seconds);
    if (seconds < 3600) return QString("%1m %2s").arg(seconds / 60).arg(seconds % 60);
    return QString("%1h %2m").arg(seconds / 3600).arg((seconds % 3600) / 60);
}

