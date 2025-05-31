#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWebEngineDownloadRequest>
#include <QTime>


class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QWebEngineDownloadRequest *download, QWidget *parent = nullptr);

    QWebEngineDownloadRequest* download() const { return m_download; }
    bool isFinished() const;

signals:
    void removeRequested(DownloadItem *item);

private slots:
    void updateProgress();
    void onStateChanged();
    void onCancelClicked();
    void onOpenFileClicked();
    void onOpenFolderClicked();

private:
    void setupUI();
    void updateButtons();
    QString formatFileSize(qint64 bytes);
    QString formatSpeed(qint64 bytesPerSecond);
    QString formatTime(int seconds);

    QWebEngineDownloadRequest *m_download;

    QLabel *m_fileNameLabel;
    QLabel *m_fileSizeLabel;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QPushButton *m_cancelButton;
    QPushButton *m_openFileButton;
    QPushButton *m_openFolderButton;
    QPushButton *m_removeButton;

    qint64 m_lastReceivedBytes;
    QTime m_lastUpdateTime;
};

#endif // DOWNLOADITEM_H

