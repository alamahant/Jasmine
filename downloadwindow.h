#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QList>

class DownloadItem;

class DownloadWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadWindow(QWidget *parent = nullptr);

    void addDownloadItem(DownloadItem *item);
    void removeDownloadItem(DownloadItem *item);
    void clearFinishedDownloads();

private slots:
    void onClearFinishedClicked();
    void onOpenDownloadsFolderClicked();
    void onDownloadItemRemoveRequested(DownloadItem *item);

private:
    void setupUI();
    void updateEmptyState();

    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollWidget;
    QVBoxLayout *m_scrollLayout;
    QLabel *m_emptyLabel;
    QPushButton *m_clearFinishedButton;
    QPushButton *m_openFolderButton;

    QList<DownloadItem*> m_downloadItems;
    QString m_downloadDirectory;


};

#endif // DOWNLOADWINDOW_H

