#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QList>
#include <QWebEngineDownloadRequest>
#include <QStandardPaths>
#include <QDir>
#include "downloadwindow.h"
class DownloadItem;
class DownloadWindow;

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    void handleDownloadRequest(QWebEngineDownloadRequest *download);
    void showDownloadsWindow();
    int activeDownloadsCount() const;
    bool hasActiveDownloads() const;


signals:
    void downloadStarted();
    void downloadFinished();
    void activeDownloadsChanged(int count);

private slots:
    void onDownloadFinished();
    void onDownloadProgress();

private:
    void setupDownloadDirectory();
    QString getDownloadPath(const QString &fileName);

    QList<QWebEngineDownloadRequest*> m_activeDownloads;
    QList<DownloadItem*> m_downloadItems;
    DownloadWindow *m_downloadWindow;
    QString m_downloadDirectory;
};

#endif // DOWNLOADMANAGER_H

