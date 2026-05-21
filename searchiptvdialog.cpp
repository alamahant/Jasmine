#include "searchiptvdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>
#include <QRegularExpression>
#include <QHeaderView>
#include<QGroupBox>
#include<QFormLayout>
#include<QProgressDialog>
#include<QUuid>
#include"Constants.h"

SearchIPTVDialog::SearchIPTVDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Search IPTV Channels");
    setMinimumSize(900, 600);
    resize(900, 600);
}

SearchIPTVDialog::~SearchIPTVDialog()
{
    if (m_parseWatcher && !m_parseWatcher->isFinished()) {
        m_parseWatcher->cancel();
        m_parseWatcher->waitForFinished();
    }
}

void SearchIPTVDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    
    // Tab widget for sources
    m_tabWidget = new QTabWidget();
    
    // ========== M3U Tab ==========
    QWidget *m3uTab = new QWidget();
    QVBoxLayout *m3uLayout = new QVBoxLayout(m3uTab);
    
    // Remote URL section
    QGroupBox *remoteGroup = new QGroupBox("Remote M3U Playlist");
    QHBoxLayout *remoteLayout = new QHBoxLayout(remoteGroup);
    m_remoteUrlEdit = new QLineEdit();
    m_remoteUrlEdit->setPlaceholderText("https://example.com/playlist.m3u8");
    m_loadRemoteButton = new QPushButton("Load");
    remoteLayout->addWidget(m_remoteUrlEdit);
    remoteLayout->addWidget(m_loadRemoteButton);
    m3uLayout->addWidget(remoteGroup);
    
    // Local file section
    QGroupBox *localGroup = new QGroupBox("Local M3U File");
    QHBoxLayout *localLayout = new QHBoxLayout(localGroup);
    m_localFileEdit = new QLineEdit();
    m_localFileEdit->setPlaceholderText("Select an M3U file...");
    m_localFileEdit->setReadOnly(true);
    m_browseButton = new QPushButton("Browse");
    m_loadLocalButton = new QPushButton("Load");
    localLayout->addWidget(m_localFileEdit);
    localLayout->addWidget(m_browseButton);
    localLayout->addWidget(m_loadLocalButton);
    m3uLayout->addWidget(localGroup);
    
    m3uLayout->addStretch();
    m_tabWidget->addTab(m3uTab, "M3U Playlist");
    
    // ========== Xtream Tab (Placeholder) ==========
    QWidget *xtreamTab = new QWidget();
    QVBoxLayout *xtreamLayout = new QVBoxLayout(xtreamTab);
    
    QGroupBox *xtreamGroup = new QGroupBox("Xtream Codes API");
    QFormLayout *xtreamForm = new QFormLayout(xtreamGroup);
    
    m_xtreamServerEdit = new QLineEdit();
    m_xtreamServerEdit->setPlaceholderText("http://panel.example.com:8080");
    m_xtreamUsernameEdit = new QLineEdit();
    m_xtreamPasswordEdit = new QLineEdit();
    m_xtreamPasswordEdit->setEchoMode(QLineEdit::Password);
    m_xtreamUserAgentEdit = new QLineEdit();
    m_xtreamUserAgentEdit->setPlaceholderText("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    xtreamForm->addRow("Server URL:", m_xtreamServerEdit);
    xtreamForm->addRow("Username:", m_xtreamUsernameEdit);
    xtreamForm->addRow("Password:", m_xtreamPasswordEdit);
    xtreamForm->addRow("User-Agent:", m_xtreamUserAgentEdit);
    
    m_xtreamConnectButton = new QPushButton("Connect & Load");
    m_xtreamConnectButton->setEnabled(false);
    m_xtreamConnectButton->setToolTip("Xtream Codes support coming soon");
    xtreamForm->addRow("", m_xtreamConnectButton);
    
    QLabel *comingSoonLabel = new QLabel("Xtream Codes support will be available in a future update.");
    comingSoonLabel->setStyleSheet("color: #808080; font-style: italic;");
    xtreamForm->addRow("", comingSoonLabel);
    
    xtreamLayout->addWidget(xtreamGroup);
    xtreamLayout->addStretch();
    m_tabWidget->addTab(xtreamTab, "Xtream Codes");
    
    mainLayout->addWidget(m_tabWidget);
    
    // ========== Filter section ==========
    QHBoxLayout *filterLayout = new QHBoxLayout();
    QLabel *filterLabel = new QLabel("Filter:");
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Type to filter channels by name or category...");
    m_searchButton = new QPushButton("Search");
    m_searchButton->setObjectName("secondaryButton");
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterEdit);
    filterLayout->addWidget(m_searchButton);
    mainLayout->addLayout(filterLayout);
    
    // ========== Results table ==========
    m_resultsTable = new QTableWidget();
    m_resultsTable->setColumnCount(5);
    m_resultsTable->setHorizontalHeaderLabels({"Select", "Channel Name", "Category", "Country", "Resolution"});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mainLayout->addWidget(m_resultsTable);
    
    // ========== Buttons ==========
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_selectAllButton = new QPushButton("Select All");
    m_selectAllButton->setObjectName("secondaryButton");
    m_previewButton = new QPushButton("Preview");
    m_previewButton->setObjectName("secondaryButton");
    m_previewButton->setEnabled(false);
    m_addButton = new QPushButton("Add Selected");
    m_addButton->setObjectName("primaryButton");
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("secondaryButton");
    
    buttonLayout->addWidget(m_selectAllButton);
    buttonLayout->addWidget(m_previewButton);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // ========== Connect signals ==========
    connect(m_loadRemoteButton, &QPushButton::clicked, this, &SearchIPTVDialog::onLoadRemoteM3U);
    connect(m_browseButton, &QPushButton::clicked, this, &SearchIPTVDialog::onBrowseLocalFile);
    connect(m_loadLocalButton, &QPushButton::clicked, this, &SearchIPTVDialog::onLoadLocalM3U);
    connect(m_searchButton, &QPushButton::clicked, this, &SearchIPTVDialog::onSearchClicked);
    connect(m_selectAllButton, &QPushButton::clicked, this, &SearchIPTVDialog::onSelectAllClicked);
    connect(m_previewButton, &QPushButton::clicked, this, &SearchIPTVDialog::onPreviewClicked);
    connect(m_addButton, &QPushButton::clicked, this, &SearchIPTVDialog::onAddSelectedClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_resultsTable, &QTableWidget::doubleClicked, this, &SearchIPTVDialog::onTableDoubleClicked);
    connect(m_xtreamConnectButton, &QPushButton::clicked, this, &SearchIPTVDialog::onXtreamConnect);
    
    // Connect table selection change to update preview button
    connect(m_resultsTable, &QTableWidget::itemSelectionChanged, this, &SearchIPTVDialog::updatePreviewButtonState);
}

void SearchIPTVDialog::onBrowseLocalFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select M3U File",
                                                    JASMINE_CONSTANTS::iptvDir,
                                                    "M3U Files (*.m3u *.m3u8);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        m_localFileEdit->setText(filePath);
    }
}

void SearchIPTVDialog::onLoadRemoteM3U()
{

    QString url = m_remoteUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a remote M3U URL.");
        return;
    }

    // Clear existing data BEFORE loading new one
    m_allChannels.clear();
    m_filteredChannels.clear();
    m_selectedRows.clear();
    m_resultsTable->clear();  // Also clears items
    m_resultsTable->setRowCount(0);  // Forces cleanup


    loadRemoteM3U(QUrl(url));
}

void SearchIPTVDialog::loadRemoteM3U(const QUrl &url)
{


    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(url));
    
    QProgressDialog *progress = new QProgressDialog("Loading remote playlist...", "Cancel", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->show();
    
    connect(reply, &QNetworkReply::finished, [this, reply, nam, progress, url]() {
        progress->accept();
        if (reply->error() == QNetworkReply::NoError) {
            QString content = QString::fromUtf8(reply->readAll());
            //parseM3U(content, url.toString());
            parseM3UAsync(content, url.toString());
        } else {
            QMessageBox::warning(this, "Error", "Failed to load M3U: " + reply->errorString());
        }
        reply->deleteLater();
        nam->deleteLater();
    });
    
    connect(progress, &QProgressDialog::canceled, [reply, nam]() {
        reply->abort();
        reply->deleteLater();
        nam->deleteLater();
    });
}

void SearchIPTVDialog::onLoadLocalM3U()
{
    QString filePath = m_localFileEdit->text().trimmed();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a local M3U file.");
        return;
    }
    loadLocalM3U(filePath);
}

void SearchIPTVDialog::loadLocalM3U(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Failed to open file: " + file.errorString());
        return;
    }
    
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    
    //parseM3U(content, filePath);
    parseM3UAsync(content, filePath);
}

void SearchIPTVDialog::parseM3U(const QString &content, const QString &sourceName)
{
    m_allChannels.clear();
    m_currentSource = sourceName;
    
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    
    QRegularExpression logoRegex("tvg-logo=\"([^\"]*)\"");
    QRegularExpression categoryRegex("group-title=\"([^\"]*)\"");
    QRegularExpression idRegex("tvg-id=\"([^\"]*)\"");
    QRegularExpression resolutionRegex("(\\d{3,4}p)");
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        
        if (line.startsWith("#EXTINF")) {
            IPTVChannel channel;
            
            // Extract logo URL
            QRegularExpressionMatch logoMatch = logoRegex.match(line);
            if (logoMatch.hasMatch()) {
                channel.logoUrl = logoMatch.captured(1);
            }
            
            // Extract category
            QRegularExpressionMatch categoryMatch = categoryRegex.match(line);
            if (categoryMatch.hasMatch()) {
                channel.category = categoryMatch.captured(1);
            }
            
            // Extract channel ID
            QRegularExpressionMatch idMatch = idRegex.match(line);
            if (idMatch.hasMatch()) {
                channel.channelId = idMatch.captured(1);
            } else {
                channel.channelId = QUuid::createUuid().toString();
            }
            
            // Extract channel name (after last comma)
            int lastComma = line.lastIndexOf(',');
            if (lastComma != -1) {
                channel.name = line.mid(lastComma + 1).trimmed();
            } else {
                channel.name = "Unknown";
            }
            
            // Extract resolution from name
            QRegularExpressionMatch resMatch = resolutionRegex.match(channel.name);
            if (resMatch.hasMatch()) {
                channel.resolution = resMatch.captured(1);
            }
            
            // Look for stream URL on next line
            if (i + 1 < lines.size()) {
                QString nextLine = lines[i + 1].trimmed();
                if (!nextLine.isEmpty() && !nextLine.startsWith("#")) {
                    channel.streamUrl = nextLine;
                }
            }
            
            if (!channel.streamUrl.isEmpty()) {
                m_allChannels.append(channel);
            }
        }
    }
    
    m_filteredChannels = m_allChannels;
    m_selectedRows.fill(false, m_filteredChannels.size());
    displayChannels();
    
    QMessageBox::information(this, "Loaded", QString("Loaded %1 channels from %2").arg(m_allChannels.size()).arg(sourceName));
}

void SearchIPTVDialog::displayChannels()
{
    m_resultsTable->setRowCount(m_filteredChannels.size());
    m_selectedRows.resize(m_filteredChannels.size());
    
    for (int i = 0; i < m_filteredChannels.size(); ++i) {
        const IPTVChannel &channel = m_filteredChannels[i];
        
        // Checkbox column
        QWidget *checkWidget = new QWidget();
        QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->setAlignment(Qt::AlignCenter);
        checkLayout->setContentsMargins(0, 0, 0, 0);
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setChecked(m_selectedRows[i]);
        checkLayout->addWidget(checkBox);
        m_resultsTable->setCellWidget(i, 0, checkWidget);
        
        connect(checkBox, &QCheckBox::toggled, [this, i](bool checked) {
            m_selectedRows[i] = checked;
        });
        
        // Other columns
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(channel.name));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(channel.category));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(channel.country));
        m_resultsTable->setItem(i, 4, new QTableWidgetItem(channel.resolution));
    }
    
    m_resultsTable->resizeColumnsToContents();
}

void SearchIPTVDialog::onFilterTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        m_filteredChannels = m_allChannels;
    } else {
        m_filteredChannels.clear();
        for (const IPTVChannel &channel : m_allChannels) {
            if (channel.name.contains(text, Qt::CaseInsensitive) ||
                channel.category.contains(text, Qt::CaseInsensitive)) {
                m_filteredChannels.append(channel);
            }
        }
    }
    m_selectedRows.fill(false, m_filteredChannels.size());
    displayChannels();
}

void SearchIPTVDialog::onSelectAllClicked()
{
    bool allSelected = true;
    for (bool selected : m_selectedRows) {
        if (!selected) {
            allSelected = false;
            break;
        }
    }
    
    bool newState = !allSelected;
    for (int i = 0; i < m_selectedRows.size(); ++i) {
        m_selectedRows[i] = newState;
    }
    displayChannels();
}

void SearchIPTVDialog::onPreviewClicked()
{
    int row = m_resultsTable->currentRow();
    if (row < 0 || row >= m_filteredChannels.size()) {
        QMessageBox::information(this, "Preview", "Please select a channel to preview.");
        return;
    }
    
    const IPTVChannel &channel = m_filteredChannels[row];
    if (!channel.streamUrl.isEmpty()) {
        emit previewChannel(channel.streamUrl, channel.name);
    }
}

void SearchIPTVDialog::onTableDoubleClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row >= 0 && row < m_filteredChannels.size()) {
        const IPTVChannel &channel = m_filteredChannels[row];
        if (!channel.streamUrl.isEmpty()) {
            emit previewChannel(channel.streamUrl, channel.name);
        }
    }
}

void SearchIPTVDialog::onAddSelectedClicked()
{
    QVector<IPTVChannel> selectedChannels;
    for (int i = 0; i < m_filteredChannels.size(); ++i) {
        if (m_selectedRows[i]) {
            selectedChannels.append(m_filteredChannels[i]);
        }
    }
    
    if (selectedChannels.isEmpty()) {
        QMessageBox::information(this, "Add Channels", "Please select at least one channel to add.");
        return;
    }
    
    emit channelsSelected(selectedChannels);
    
    // Clear selection after adding
    for (int i = 0; i < m_selectedRows.size(); ++i) {
        m_selectedRows[i] = false;
    }
    displayChannels();
    
    //statusBar()->showMessage(QString("Added %1 channel(s). You can load another playlist or add more.").arg(selectedChannels.size()), 3000);
}

void SearchIPTVDialog::updatePreviewButtonState()
{
    m_previewButton->setEnabled(m_resultsTable->currentRow() >= 0);
}

void SearchIPTVDialog::onXtreamConnect()
{
    QMessageBox::information(this, "Coming Soon", "Xtream Codes support will be available in a future update.\n\nFor now, please use M3U playlists.");
}


// concurrent parsing

void SearchIPTVDialog::parseM3UAsync(const QString &content, const QString &sourceName)
{
    // Show parsing progress dialog
    QProgressDialog *progress = new QProgressDialog("Parsing playlist...", "Cancel", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->show();

    // Run parsing in background thread
    QFuture<QVector<IPTVChannel>> future = QtConcurrent::run([content, sourceName]() {
        QVector<IPTVChannel> channels;
        QStringList lines = content.split('\n', Qt::SkipEmptyParts);

        QRegularExpression logoRegex("tvg-logo=\"([^\"]*)\"");
        QRegularExpression categoryRegex("group-title=\"([^\"]*)\"");
        QRegularExpression idRegex("tvg-id=\"([^\"]*)\"");
        QRegularExpression resolutionRegex("(\\d{3,4}p|\\d{3,4}i)");

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i].trimmed();

            if (line.startsWith("#EXTINF")) {
                IPTVChannel channel;

                QRegularExpressionMatch logoMatch = logoRegex.match(line);
                if (logoMatch.hasMatch()) {
                    channel.logoUrl = logoMatch.captured(1);
                }

                QRegularExpressionMatch categoryMatch = categoryRegex.match(line);
                if (categoryMatch.hasMatch()) {
                    channel.category = categoryMatch.captured(1);
                }

                QRegularExpressionMatch idMatch = idRegex.match(line);
                if (idMatch.hasMatch()) {
                    channel.channelId = idMatch.captured(1);
                } else {
                    channel.channelId = QUuid::createUuid().toString();
                }

                int lastComma = line.lastIndexOf(',');
                if (lastComma != -1) {
                    channel.name = line.mid(lastComma + 1).trimmed();
                } else {
                    channel.name = "Unknown";
                }

                QRegularExpressionMatch resMatch = resolutionRegex.match(channel.name);
                if (resMatch.hasMatch()) {
                    channel.resolution = resMatch.captured(1);
                }

                if (i + 1 < lines.size()) {
                    QString nextLine = lines[i + 1].trimmed();
                    if (!nextLine.isEmpty() && !nextLine.startsWith("#")) {
                        channel.streamUrl = nextLine;
                    }
                }

                if (!channel.streamUrl.isEmpty()) {
                    channels.append(channel);
                }
            }
        }

        return channels;
    });

    // Watch for completion
    m_parseWatcher = new QFutureWatcher<QVector<IPTVChannel>>(this);
    connect(m_parseWatcher, &QFutureWatcher<QVector<IPTVChannel>>::finished, this, &SearchIPTVDialog::onParseFinished);
    connect(progress, &QProgressDialog::canceled, [this]() {
        if (m_parseWatcher && !m_parseWatcher->isFinished()) {
            m_parseWatcher->cancel();
        }
    });

    m_parseWatcher->setFuture(future);

    // Store source name for later
    m_currentSource = sourceName;

    // Clean up progress dialog when done
    connect(m_parseWatcher, &QFutureWatcher<QVector<IPTVChannel>>::finished, progress, &QProgressDialog::accept);
    connect(m_parseWatcher, &QFutureWatcher<QVector<IPTVChannel>>::finished, progress, &QProgressDialog::deleteLater);
}

void SearchIPTVDialog::onParseFinished()
{
    if (!m_parseWatcher) return;

    if (m_parseWatcher->isCanceled()) {
        QMessageBox::information(this, "Cancelled", "Parsing was cancelled.");
        return;
    }

    m_allChannels = m_parseWatcher->result();
    m_filteredChannels = m_allChannels;
    m_selectedRows.fill(false, m_filteredChannels.size());
    displayChannels();

    QString msg;
    if (m_allChannels.size() > 1000) {
        msg = QString("Loaded %1 channels.\n\n"
                      "Note: Large playlists may affect performance.\n"
                      "We recommend adding your desired channels now, then restarting Jasmine\n"
                      "for optimal memory usage.\n\n"
                      "This dialog stays open so you won't need to reload the playlist.")
                .arg(m_allChannels.size());
    } else {
        msg = QString("Loaded %1 channels from %2").arg(m_allChannels.size()).arg(m_currentSource);
    }

    QMessageBox::information(this, "Loaded", msg);

    //QMessageBox::information(this, "Loaded", QString("Loaded %1 channels from %2").arg(m_allChannels.size()).arg(m_currentSource));
}

void SearchIPTVDialog::onSearchClicked()
{
    QString searchText = m_filterEdit->text().trimmed();

    // Show progress dialog
    QProgressDialog *progress = new QProgressDialog("Filtering channels...", "Cancel", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(200);
    progress->show();

    // Run filtering in background
    QFuture<QVector<IPTVChannel>> future = QtConcurrent::run([this, searchText]() {
        QVector<IPTVChannel> filtered;
        if (searchText.isEmpty()) {
            return m_allChannels;
        }
        for (const IPTVChannel &channel : m_allChannels) {
            if (channel.name.contains(searchText, Qt::CaseInsensitive) ||
                channel.category.contains(searchText, Qt::CaseInsensitive) ||
                channel.country.contains(searchText, Qt::CaseInsensitive)) {
                filtered.append(channel);
            }
        }
        return filtered;
    });

    QFutureWatcher<QVector<IPTVChannel>> *watcher = new QFutureWatcher<QVector<IPTVChannel>>(this);
    connect(watcher, &QFutureWatcher<QVector<IPTVChannel>>::finished, [this, watcher, progress]() {
        m_filteredChannels = watcher->result();
        m_selectedRows.fill(false, m_filteredChannels.size());
        displayChannels();
        progress->accept();
        watcher->deleteLater();
        progress->deleteLater();

        // Show result count in status bar
        if (m_filteredChannels.size() != m_allChannels.size()) {
            //statusBar()->showMessage(QString("Found %1 matching channels").arg(m_filteredChannels.size()), 3000);
        }
    });
    connect(progress, &QProgressDialog::canceled, [watcher]() {
        if (watcher && !watcher->isFinished()) {
            watcher->cancel();
        }
    });
    watcher->setFuture(future);
}
