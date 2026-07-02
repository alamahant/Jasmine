#include "historymanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QDir>

HistoryManager::HistoryManager(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    loadHistory();
    setupUI();
    populateHistory();
}

HistoryManager::~HistoryManager()
{
    saveHistory();
}

void HistoryManager::setupUI()
{
    setWindowTitle("History");
    setMinimumSize(700, 450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Search and filter
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Search:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Filter history...");
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(new QLabel("Date:"));
    m_dateFilter = new QDateEdit();
    m_dateFilter->setCalendarPopup(true);
    m_dateFilter->setDate(QDate::currentDate());
    m_dateFilter->setSpecialValueText("All");
    filterLayout->addWidget(m_dateFilter);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // List
    m_historyList = new QListWidget();
    m_historyList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_historyList->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(m_historyList);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_launchButton = new QPushButton("Launch");
    m_launchButton->setObjectName("primaryButton");
    m_deleteButton = new QPushButton("Delete Selected");
    m_deleteButton->setObjectName("secondaryButton");
    m_clearButton = new QPushButton("Clear All");
    m_clearButton->setObjectName("secondaryButton");
    QPushButton* closeBtn = new QPushButton("Close");

    buttonLayout->addWidget(m_launchButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &HistoryManager::onSearchTextChanged);
    connect(m_dateFilter, &QDateEdit::dateChanged, this, &HistoryManager::onFilterChanged);
    connect(m_clearButton, &QPushButton::clicked, this, &HistoryManager::onClearHistory);
    connect(m_deleteButton, &QPushButton::clicked, this, &HistoryManager::onDeleteSelected);
    connect(m_launchButton, &QPushButton::clicked, this, &HistoryManager::onLaunchSelected);
    connect(closeBtn, &QPushButton::clicked, this, &HistoryManager::accept);
    connect(m_historyList, &QListWidget::customContextMenuRequested, 
            this, &HistoryManager::showContextMenu);
    connect(m_historyList, &QListWidget::itemDoubleClicked, this, &HistoryManager::onLaunchSelected);
}


void HistoryManager::addEntry(const QString& url, const QString& title, const QIcon& favicon)
{
    if (url.isEmpty() || url == "about:blank") return;

    // Normalize URL - remove trailing slashes and protocol
    QString normalizedUrl = url;
    normalizedUrl.remove("https://");
    normalizedUrl.remove("http://");
    normalizedUrl.remove("www.");
    if (normalizedUrl.endsWith('/')) {
        normalizedUrl.chop(1);
    }

    // Check if normalized URL already exists
    for (int i = 0; i < m_history.size(); ++i) {
        QString existing = m_history[i].url;
        existing.remove("https://");
        existing.remove("http://");
        existing.remove("www.");
        if (existing.endsWith('/')) {
            existing.chop(1);
        }

        if (existing == normalizedUrl) {
            // Update existing entry
            m_history[i].visitCount++;
            m_history[i].visitTime = QDateTime::currentDateTime();
            if (!favicon.isNull()) {
                m_history[i].favicon = favicon;
            }
            if (!title.isEmpty()) {
                m_history[i].title = title;
            }
            // Move to front
            HistoryEntry entry = m_history.takeAt(i);
            m_history.prepend(entry);
            saveHistory();
            emit historyChanged();
            populateHistory();
            return;
        }
    }

    // Add new entry with original URL
    HistoryEntry entry(url, title.isEmpty() ? url : title);
    entry.favicon = favicon;
    m_history.prepend(entry);

    while (m_history.size() > MAX_HISTORY_ENTRIES) {
        m_history.removeLast();
    }

    saveHistory();
    emit historyChanged();
    populateHistory();
}

void HistoryManager::clearHistory()
{
    m_history.clear();
    saveHistory();
    emit historyChanged();
    populateHistory();
}

void HistoryManager::removeEntry(const QString& url)
{
    for (int i = 0; i < m_history.size(); ++i) {
        if (m_history[i].url == url) {
            m_history.removeAt(i);
            saveHistory();
            emit historyChanged();
            populateHistory();
            return;
        }
    }
}

QList<HistoryEntry> HistoryManager::getHistory() const
{
    return m_history;
}

QList<HistoryEntry> HistoryManager::getTodayHistory() const
{
    QList<HistoryEntry> result;
    QDate today = QDate::currentDate();
    for (const HistoryEntry& entry : m_history) {
        if (entry.visitTime.date() == today) {
            result.append(entry);
        }
    }
    return result;
}

QList<HistoryEntry> HistoryManager::getHistoryForDate(const QDate& date) const
{
    QList<HistoryEntry> result;
    for (const HistoryEntry& entry : m_history) {
        if (entry.visitTime.date() == date) {
            result.append(entry);
        }
    }
    return result;
}

QList<HistoryEntry> HistoryManager::searchHistory(const QString& query) const
{
    QList<HistoryEntry> result;
    QString lowerQuery = query.toLower();
    for (const HistoryEntry& entry : m_history) {
        if (entry.title.toLower().contains(lowerQuery) || 
            entry.url.toLower().contains(lowerQuery)) {
            result.append(entry);
        }
    }
    return result;
}

void HistoryManager::onClearHistory()
{
    if (QMessageBox::question(this, "Clear History", 
                              "Are you sure you want to clear all history?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        clearHistory();
    }
}


void HistoryManager::onDeleteSelected()
{
    QList<QListWidgetItem*> selected = m_historyList->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select items to delete.");
        return;
    }

    if (QMessageBox::question(this, "Delete History",
                              QString("Delete %1 selected items?").arg(selected.size()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        // Collect URLs first, then delete
        QStringList urlsToDelete;
        for (QListWidgetItem* item : selected) {
            urlsToDelete.append(item->data(Qt::UserRole).toString());
        }

        // Delete after collecting
        for (const QString& url : urlsToDelete) {
            removeEntry(url);
        }

        // Clear selection and form

        populateHistory();
    }
}

void HistoryManager::onLaunchSelected()
{
    QList<QListWidgetItem*> selected = m_historyList->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select an item to launch.");
        return;
    }
    
    for (QListWidgetItem* item : selected) {
        QString url = item->data(Qt::UserRole).toString();
        if (!url.isEmpty()) {
            emit urlLaunched(url);
        }
    }
    accept();
}

void HistoryManager::onFilterChanged()
{
    populateHistory();
}

void HistoryManager::onSearchTextChanged(const QString& text)
{
    populateHistory();
}

void HistoryManager::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_historyList->itemAt(pos);
    if (!item) {
        QMenu menu;
        QAction* clearAllAction = menu.addAction("Clear All History");
        if (menu.exec(m_historyList->mapToGlobal(pos)) == clearAllAction) {
            onClearHistory();
        }
        return;
    }
    
    QMenu menu;
    QAction* launchAction = menu.addAction("Launch");
    QAction* launchNewTabAction = menu.addAction("Launch in New Tab");
    QAction* copyAction = menu.addAction("Copy URL");
    QAction* deleteAction = menu.addAction("Delete");
    menu.addSeparator();
    QAction* clearAction = menu.addAction("Clear All");
    
    QAction* selected = menu.exec(m_historyList->mapToGlobal(pos));
    if (!selected) return;
    
    QString url = item->data(Qt::UserRole).toString();
    if (selected == launchAction || selected == launchNewTabAction) {
        emit urlLaunched(url);
        if (selected == launchAction) accept();
    } else if (selected == copyAction) {
        QApplication::clipboard()->setText(url);
    } else if (selected == deleteAction) {
        removeEntry(url);
    } else if (selected == clearAction) {
        onClearHistory();
    }
}

void HistoryManager::populateHistory()
{
    m_historyList->clear();
    QDate date = m_dateFilter->date();
    QString search = m_searchEdit->text().trimmed().toLower();
    
    QList<HistoryEntry> entries;
    if (date == m_dateFilter->minimumDate()) {
        entries = getHistory();
    } else {
        entries = getHistoryForDate(date);
    }
    
    std::sort(entries.begin(), entries.end(), 
        [](const HistoryEntry& a, const HistoryEntry& b) {
            return a.visitTime > b.visitTime;
        });
    
    for (const HistoryEntry& entry : entries) {
        if (!search.isEmpty()) {
            if (!entry.title.toLower().contains(search) && 
                !entry.url.toLower().contains(search)) {
                continue;
            }
        }
        
        QString displayText = QString("%1 - %2")
            .arg(entry.visitTime.toString("yyyy-MM-dd hh:mm"))
            .arg(entry.title.isEmpty() ? entry.url : entry.title);
        
        if (entry.visitCount > 1) {
            displayText += QString(" (%1 visits)").arg(entry.visitCount);
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, entry.url);
        if (!entry.favicon.isNull()) {
            item->setIcon(entry.favicon);
        }
        m_historyList->addItem(item);
    }
    
    m_historyList->setEnabled(m_historyList->count() > 0);
    m_deleteButton->setEnabled(m_historyList->count() > 0);
    m_clearButton->setEnabled(m_historyList->count() > 0);
}

bool HistoryManager::loadHistory()
{
    QString filePath = getHistoryFilePath();
    QFile file(filePath);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;
    
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);
    
    quint32 count;
    in >> count;
    m_history.clear();
    for (quint32 i = 0; i < count; ++i) {
        HistoryEntry entry;
        in >> entry.url;
        in >> entry.title;
        in >> entry.visitTime;
        in >> entry.visitCount;
        m_history.append(entry);
    }
    file.close();
    return true;
}

bool HistoryManager::saveHistory()
{
    QString filePath = getHistoryFilePath();
    QDir().mkpath(QFileInfo(filePath).dir().path());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(m_history.size());
    for (const HistoryEntry& entry : m_history) {
        out << entry.url;
        out << entry.title;
        out << entry.visitTime;
        out << entry.visitCount;
    }
    file.close();
    return true;
}

QString HistoryManager::getHistoryFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/history.dat";
}

void HistoryManager::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}
