#include "downloadmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include "downloaditem.h"
#include <QApplication>


DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , m_downloadWindow(nullptr)
{
    setupDownloadDirectory();
    m_downloadWindow = new DownloadWindow();
    m_downloadWindow->hide();

}

DownloadManager::~DownloadManager()
{
    // Clean up any active downloads
    for (auto download : m_activeDownloads) {
        if (download && download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
            download->cancel();
        }
    }
    if(m_downloadWindow){
        m_downloadWindow->close();
        m_downloadWindow->deleteLater();
        m_downloadWindow = nullptr;
    }
}

void DownloadManager::setupDownloadDirectory()
{
#ifdef FLATPAK_BUILD
    m_downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Downloads";
#else
    m_downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Jasmine";
#endif
    QDir dir;
    if (!dir.exists(m_downloadDirectory)) {
        dir.mkpath(m_downloadDirectory);
    }
}

void DownloadManager::handleDownloadRequest(QWebEngineDownloadRequest *download)
{
    if (!download) {
        return;
    }

    // Set download path
    QString fileName = QFileInfo(download->downloadFileName()).fileName();
    if (fileName.isEmpty()) {
        fileName = "download";
    }

    QString filePath = getDownloadPath(fileName);
    download->setDownloadDirectory(QFileInfo(filePath).absolutePath());
    download->setDownloadFileName(QFileInfo(filePath).fileName());

    // Connect signals
    connect(download, &QWebEngineDownloadRequest::isFinishedChanged,
            this, &DownloadManager::onDownloadFinished);
    connect(download, &QWebEngineDownloadRequest::receivedBytesChanged,
            this, &DownloadManager::onDownloadProgress);

    // Accept and start download
    download->accept();

    // Track the download
    m_activeDownloads.append(download);

    // CREATE AND ADD DOWNLOAD ITEM TO WINDOW
    DownloadItem *item = new DownloadItem(download, nullptr);
    m_downloadItems.append(item);


    if (m_downloadWindow) {
        m_downloadWindow->addDownloadItem(item);
    }

    emit downloadStarted();
    emit activeDownloadsChanged(activeDownloadsCount());
}

void DownloadManager::onDownloadFinished()
{
    QWebEngineDownloadRequest *download = qobject_cast<QWebEngineDownloadRequest*>(sender());
    if (!download) {
        return;
    }

    m_activeDownloads.removeAll(download);


    emit downloadFinished();
    emit activeDownloadsChanged(activeDownloadsCount());
}

void DownloadManager::onDownloadProgress()
{
    QWebEngineDownloadRequest *download = qobject_cast<QWebEngineDownloadRequest*>(sender());
    if (!download) {
        return;
    }

    qint64 bytesReceived = download->receivedBytes();
    qint64 bytesTotal = download->totalBytes();

    if (bytesTotal > 0) {
        int progress = (bytesReceived * 100) / bytesTotal;
    }
}

QString DownloadManager::getDownloadPath(const QString &fileName)
{
    QString basePath = m_downloadDirectory + "/" + fileName;
    QString finalPath = basePath;

    // Handle duplicate filenames
    int counter = 1;
    while (QFile::exists(finalPath)) {
        QFileInfo info(basePath);
        finalPath = QString("%1/%2 (%3).%4")
                        .arg(m_downloadDirectory)
                        .arg(info.baseName())
                        .arg(counter)
                        .arg(info.suffix());
        counter++;
    }

    return finalPath;
}


void DownloadManager::showDownloadsWindow(){
    if (m_downloadWindow) {
        // Apply current main window stylesheet to download window
        m_downloadWindow->setStyleSheet(qApp->activeWindow()->styleSheet());
        m_downloadWindow->show();
        m_downloadWindow->raise();
        m_downloadWindow->activateWindow();
    }
}

int DownloadManager::activeDownloadsCount() const
{
    return m_activeDownloads.count();
}

bool DownloadManager::hasActiveDownloads() const
{
    return !m_activeDownloads.isEmpty();
}


