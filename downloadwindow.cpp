#include "downloadwindow.h"
#include "downloaditem.h"
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include<QDir>
#include<QMessageBox>
#include <QApplication>
#include <QClipboard>

DownloadWindow::DownloadWindow(QWidget *parent)
    : QDialog(parent)
{
#ifdef FLATPAK_BUILD
    m_downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Downloads";
#else
    m_downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Downloads";
#endif
    setupUI();
    updateEmptyState();
}

void DownloadWindow::setupUI()
{
    setWindowTitle("Downloads");
    setMinimumSize(500, 400);

#ifdef FLATPAK_BUILD
    resize(800, 500);  // Wider for Flatpak
#else
    resize(600, 500);  // Normal for desktop
#endif

    //resize(600, 500);

    m_mainLayout = new QVBoxLayout(this);

    // Top buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_clearFinishedButton = new QPushButton("Clear Finished");
    m_openFolderButton = new QPushButton("Open Downloads Folder");

    buttonLayout->addWidget(m_clearFinishedButton);
    buttonLayout->addWidget(m_openFolderButton);
    buttonLayout->addStretch();

    m_mainLayout->addLayout(buttonLayout);
    // Scroll area for download items
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_scrollWidget = new QWidget();
    m_scrollLayout = new QVBoxLayout(m_scrollWidget);
    m_scrollLayout->setAlignment(Qt::AlignTop);
    m_scrollLayout->setSpacing(5);

    m_scrollArea->setWidget(m_scrollWidget);
    m_mainLayout->addWidget(m_scrollArea);

    // Empty state label
    m_emptyLabel = new QLabel("No downloads yet");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: gray; font-size: 14px; padding: 50px;");
    m_scrollLayout->addWidget(m_emptyLabel);

    // Connect button signals
    connect(m_clearFinishedButton, &QPushButton::clicked, this, &DownloadWindow::onClearFinishedClicked);
    connect(m_openFolderButton, &QPushButton::clicked, this, &DownloadWindow::onOpenDownloadsFolderClicked);
}

void DownloadWindow::addDownloadItem(DownloadItem *item)
{
    if (!item || m_downloadItems.contains(item)) {

        return;
    }

    m_downloadItems.append(item);
    m_scrollLayout->addWidget(item);

    // Connect remove signal
    connect(item, &DownloadItem::removeRequested, this, &DownloadWindow::onDownloadItemRemoveRequested);

    updateEmptyState();
}

void DownloadWindow::removeDownloadItem(DownloadItem *item)
{
    if (!item || !m_downloadItems.contains(item)) {
        return;
    }

    m_downloadItems.removeAll(item);
    m_scrollLayout->removeWidget(item);
    item->deleteLater();

    updateEmptyState();
}

void DownloadWindow::clearFinishedDownloads()
{
    QList<DownloadItem*> itemsToRemove;

    for (DownloadItem *item : m_downloadItems) {
        if (item->isFinished()) {
            itemsToRemove.append(item);
        }
    }

    for (DownloadItem *item : itemsToRemove) {
        removeDownloadItem(item);
    }
}

void DownloadWindow::onClearFinishedClicked()
{
    clearFinishedDownloads();
}
/*
void DownloadWindow::onOpenDownloadsFolderClicked()
{
    QDir downloadsDir(m_downloadDirectory);

    // Create directory if it doesn't exist
    if (!downloadsDir.exists()) {
        downloadsDir.mkpath(".");
    }

#ifdef FLATPAK_BUILD
    // Show helpful message for Flathub users
    QMessageBox::information(this, "Downloads Location",
                             QString("Your downloads are saved to:\n\n%1\n\n"
                                     "You can access this folder using your system's file manager.")
                                 .arg(m_downloadDirectory));
#else
    // Open in system file manager
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadDirectory));
#endif

}
*/

void DownloadWindow::onOpenDownloadsFolderClicked(){
    QDir downloadsDir(m_downloadDirectory);
    // Create directory if it doesn't exist
    if (!downloadsDir.exists()) {
        downloadsDir.mkpath(".");
    }
#ifdef FLATPAK_BUILD
    // Show helpful message for Flatpak users
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Downloads Location");
    msgBox.setText(QString("Your downloads are saved to:\n\n%1\n\n"
                           "You can access this folder using your system's file manager.")
                       .arg(m_downloadDirectory));
    msgBox.setStandardButtons(QMessageBox::Ok);
    QPushButton *copyButton = msgBox.addButton("Copy Path", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QApplication::clipboard()->setText(m_downloadDirectory);
    }
#else
    // Open in system file manager
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadDirectory));
#endif
}



void DownloadWindow::onDownloadItemRemoveRequested(DownloadItem *item)
{
    removeDownloadItem(item);
}

void DownloadWindow::updateEmptyState()
{
    bool isEmpty = m_downloadItems.isEmpty();
    m_emptyLabel->setVisible(isEmpty);
    m_clearFinishedButton->setEnabled(!isEmpty);
}

