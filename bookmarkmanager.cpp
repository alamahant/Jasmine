#include "bookmarkmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
BookmarkManager::BookmarkManager(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    //m_folders << "Default" << "Favorites" << "Reading List";
    if (m_folders.isEmpty()) {
        m_folders << "Default" << "Favorites" << "Reading List";
    }
    loadBookmarks();

    setupUI();
    populateFolders();
    populateBookmarks();
    clearForm();
    m_folderCombo->setCurrentIndex(0);
    if (m_bookmarkList->count() > 0) {
        m_bookmarkList->setCurrentRow(0);
        onBookmarkSelected();
    }
    onBookmarkSelected();
}

BookmarkManager::~BookmarkManager()
{
    saveBookmarks();
}

void BookmarkManager::setupUI()
{
    setWindowTitle("Bookmark Manager");
    setMinimumSize(750, 550);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    QPushButton* addBookmarkBtn = new QPushButton("Add Bookmark");
    addBookmarkBtn->setObjectName("primaryButton");
    QPushButton* launchBtn = new QPushButton("Launch");
    launchBtn->setObjectName("primaryButton");
    QPushButton* closeBtn = new QPushButton("Close");

    toolbarLayout->addWidget(addBookmarkBtn);
    toolbarLayout->addWidget(launchBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(closeBtn);
    mainLayout->addLayout(toolbarLayout);

    QHBoxLayout* splitLayout = new QHBoxLayout();

    QGroupBox* listGroup = new QGroupBox("Bookmarks");
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Search:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Filter bookmarks...");
    searchLayout->addWidget(m_searchEdit);
    listLayout->addLayout(searchLayout);

    QHBoxLayout* folderLayout = new QHBoxLayout();
    folderLayout->addWidget(new QLabel("Folder:"));
    m_folderCombo = new QComboBox();
    folderLayout->addWidget(m_folderCombo);
    m_addFolderButton = new QPushButton("+");
    m_addFolderButton->setFixedSize(25, 25);
    m_addFolderButton->setStyleSheet(
        "QPushButton {"
        "color: black;"
        "background: white;"
        "border: 1px solid gray;"
        "font-size: 18px;"
        "font-weight: bold;"
        "padding: 0px;"
        "}"
    );
    m_deleteFolderButton = new QPushButton("-");
    m_deleteFolderButton->setFixedSize(25, 25);
    m_deleteFolderButton->setStyleSheet(
        "QPushButton {"
        "color: black;"
        "background: white;"
        "border: 1px solid gray;"
        "font-size: 18px;"
        "font-weight: bold;"
        "padding: 0px;"
        "}"
    );
    folderLayout->addWidget(m_addFolderButton);
    folderLayout->addWidget(m_deleteFolderButton);
    folderLayout->addStretch();
    listLayout->addLayout(folderLayout);

    m_bookmarkList = new QListWidget();
    m_bookmarkList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listLayout->addWidget(m_bookmarkList);

    splitLayout->addWidget(listGroup, 2);

    QGroupBox* detailsGroup = new QGroupBox("Details");
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsGroup);

    QFormLayout* formLayout = new QFormLayout();
    m_titleEdit = new QLineEdit();
    m_urlEdit = new QLineEdit();
    m_notesEdit = new QTextEdit();
    m_notesEdit->setMaximumHeight(80);

    formLayout->addRow("Title:", m_titleEdit);
    formLayout->addRow("URL:", m_urlEdit);
    formLayout->addRow("Notes:", m_notesEdit);
    detailsLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Add");
    m_addButton->setObjectName("primaryButton");
    m_editButton = new QPushButton("Update");
    m_editButton->setObjectName("secondaryButton");
    m_deleteButton = new QPushButton("Delete");
    m_deleteButton->setObjectName("secondaryButton");
    m_launchButton = new QPushButton("Launch");
    m_launchButton->setObjectName("primaryButton");
    QPushButton* clearButton = new QPushButton("Clear");
    clearButton->setObjectName("secondaryButton");

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_launchButton);
    buttonLayout->addWidget(clearButton);
    detailsLayout->addLayout(buttonLayout);

    splitLayout->addWidget(detailsGroup, 1);
    mainLayout->addLayout(splitLayout);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &BookmarkManager::onSearchTextChanged);
    connect(m_bookmarkList, &QListWidget::itemClicked, this, &BookmarkManager::onBookmarkSelected);
    connect(m_addButton, &QPushButton::clicked, this, &BookmarkManager::onAddBookmark);
    connect(m_editButton, &QPushButton::clicked, this, &BookmarkManager::onEditBookmark);
    connect(m_deleteButton, &QPushButton::clicked, this, &BookmarkManager::onDeleteBookmark);
    connect(m_launchButton, &QPushButton::clicked, this, &BookmarkManager::onLaunchBookmark);
    connect(clearButton, &QPushButton::clicked, this, &BookmarkManager::onClearForm);
    connect(m_folderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this]() { onFolderChanged(m_folderCombo->currentText()); });
    connect(m_addFolderButton, &QPushButton::clicked, this, &BookmarkManager::onAddFolder);
    connect(m_deleteFolderButton, &QPushButton::clicked, this, &BookmarkManager::onDeleteFolder);
    connect(addBookmarkBtn, &QPushButton::clicked, this, &BookmarkManager::onAddBookmark);
    connect(launchBtn, &QPushButton::clicked, this, &BookmarkManager::onLaunchBookmark);
    connect(closeBtn, &QPushButton::clicked, this, &BookmarkManager::accept);
    connect(m_bookmarkList, &QListWidget::itemDoubleClicked, this, &BookmarkManager::onLaunchBookmark);
}

void BookmarkManager::addBookmark(const Bookmark& bookmark)
{
    m_bookmarks.append(bookmark);
    saveBookmarks();
    emit bookmarksChanged();
    populateBookmarks();
}

void BookmarkManager::removeBookmark(const QString& id)
{
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (m_bookmarks[i].id == id) {
            m_bookmarks.removeAt(i);
            saveBookmarks();
            emit bookmarksChanged();
            populateBookmarks();
            clearForm();
            return;
        }
    }
}

void BookmarkManager::updateBookmark(const Bookmark& bookmark)
{
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (m_bookmarks[i].id == bookmark.id) {
            m_bookmarks[i] = bookmark;
            saveBookmarks();
            emit bookmarksChanged();
            populateBookmarks();
            clearForm();
            return;
        }
    }
}

QList<Bookmark> BookmarkManager::getBookmarks() const
{
    return m_bookmarks;
}

QList<Bookmark> BookmarkManager::getBookmarksInFolder(const QString& folder) const
{
    QList<Bookmark> result;
    for (const Bookmark& b : m_bookmarks) {
        if (b.folder == folder) {
            result.append(b);
        }
    }
    return result;
}

QStringList BookmarkManager::getFolders() const
{
    return m_folders;
}

void BookmarkManager::addFolder(const QString& folderName)
{
    if (!m_folders.contains(folderName)) {
        m_folders.append(folderName);
        saveBookmarks();
        populateFolders();
    }
}

void BookmarkManager::removeFolder(const QString& folderName)
{
    if (folderName == "Default") return;
    m_folders.removeAll(folderName);

    for (Bookmark& b : m_bookmarks) {
        if (b.folder == folderName) {
            b.folder = "Default";
        }
    }
    saveBookmarks();
    emit bookmarksChanged();
    populateFolders();
    populateBookmarks();
}

void BookmarkManager::moveBookmarkToFolder(const QString& bookmarkId, const QString& folderName)
{
    for (Bookmark& b : m_bookmarks) {
        if (b.id == bookmarkId) {
            b.folder = folderName;
            saveBookmarks();
            emit bookmarksChanged();
            populateBookmarks();
            return;
        }
    }
}

void BookmarkManager::onAddBookmark()
{
    QString title = m_titleEdit->text().trimmed();
    QString url = m_urlEdit->text().trimmed();

    if (url.isEmpty()) {
        QMessageBox::warning(this, "Invalid URL", "Please enter a URL.");
        return;
    }
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
    }
    if (title.isEmpty()) {
        title = url;
    }

    for (const Bookmark& existing : m_bookmarks) {
        if (existing.url == url) {
            QMessageBox::information(this, "Duplicate Bookmark",
                                     "This URL is already in your bookmarks.");

            for (int i = 0; i < m_bookmarkList->count(); ++i) {
                        QListWidgetItem* item = m_bookmarkList->item(i);
                        if (item->data(Qt::UserRole).toString() == existing.id) {
                            m_bookmarkList->setCurrentItem(item);
                            m_bookmarkList->scrollToItem(item);

                            break;
                        }
                    }
            return;
        }
    }

    Bookmark b;
    b.id = QUuid::createUuid().toString();
    b.title = title;
    b.url = url;
    b.notes = m_notesEdit->toPlainText();
    b.folder = m_folderCombo->currentText();
    if (b.folder == "All") b.folder = "Default";

    addBookmark(b);
    clearForm();
}

void BookmarkManager::onEditBookmark()
{
    if (m_selectedId.isEmpty()) return;

    QList<Bookmark> bookmarks = getBookmarks();
    for (Bookmark& b : bookmarks) {
        if (b.id == m_selectedId) {
            b.title = m_titleEdit->text().trimmed();
            b.url = m_urlEdit->text().trimmed();
            b.notes = m_notesEdit->toPlainText();
            if (b.title.isEmpty()) b.title = b.url;
            updateBookmark(b);
            return;
        }
    }
}



void BookmarkManager::onDeleteBookmark()
{
    QList<QListWidgetItem*> selected = m_bookmarkList->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a bookmark to delete.");
        return;
    }

    if (selected.size() > 1) {
        if (QMessageBox::question(this, "Delete Bookmarks",
                                  QString("Delete %1 selected bookmarks?").arg(selected.size()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            QStringList idsToDelete;
            for (QListWidgetItem* item : selected) {
                idsToDelete.append(item->data(Qt::UserRole).toString());
            }

            for (const QString& id : idsToDelete) {
                removeBookmark(id);
            }
            clearForm();
            m_selectedId.clear();
        }
        return;
    }

    // Single selection
    if (m_selectedId.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a bookmark to delete.");
        return;
    }

    if (QMessageBox::question(this, "Delete Bookmark",
                              "Are you sure you want to delete this bookmark?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        removeBookmark(m_selectedId);
        m_selectedId.clear();
    }
}

void BookmarkManager::onFolderChanged(const QString& folder)
{
    populateBookmarks();
}

void BookmarkManager::onBookmarkSelected()
{
    QListWidgetItem* item = m_bookmarkList->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    QList<Bookmark> bookmarks = getBookmarks();
    for (const Bookmark& b : bookmarks) {
        if (b.id == id) {
            showBookmarkDetails(b);
            return;
        }
    }
}

void BookmarkManager::onLaunchBookmark()
{
    QList<QListWidgetItem*> selected = m_bookmarkList->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a bookmark to launch.");
        return;
    }

    for (QListWidgetItem* item : selected) {
        QString id = item->data(Qt::UserRole).toString();
        QList<Bookmark> bookmarks = getBookmarks();
        for (const Bookmark& b : bookmarks) {
            if (b.id == id) {
                emit urlLaunched(b.url);
                break;
            }
        }
    }
    accept();
}

void BookmarkManager::onAddFolder()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Add Folder",
                                         "Enter folder name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        addFolder(name);
        int index = m_folderCombo->findText(name);
        if (index >= 0) m_folderCombo->setCurrentIndex(index);
    }
}

void BookmarkManager::onDeleteFolder()
{
    QString folder = m_folderCombo->currentText();
    if (folder == "All" || folder == "Default" || folder == "Favorites") {
        QMessageBox::information(this, "Cannot Delete",
                                 "Cannot delete the 'All', 'Default' or 'Favorites' folder.");
        return;
    }
    if (QMessageBox::question(this, "Delete Folder",
                              QString("Delete folder '%1'? Bookmarks will be moved to 'Default'.").arg(folder),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        removeFolder(folder);
        m_folderCombo->setCurrentIndex(0);
    }
}

void BookmarkManager::onClearForm()
{
    clearForm();
}

void BookmarkManager::onSearchTextChanged(const QString& text)
{
    populateBookmarks();
}

void BookmarkManager::populateBookmarks()
{
    m_bookmarkList->clear();
    QString currentFolder = m_folderCombo->currentText();
    QString searchText = m_searchEdit->text().toLower();

    QList<Bookmark> bookmarks;
    if (currentFolder.isEmpty() || currentFolder == "All") {
        bookmarks = getBookmarks();
    } else {
        bookmarks = getBookmarksInFolder(currentFolder);
    }

    for (const Bookmark& b : bookmarks) {
        if (!searchText.isEmpty()) {
            if (!b.title.toLower().contains(searchText) &&
                !b.url.toLower().contains(searchText) &&
                !b.notes.toLower().contains(searchText)) {
                continue;
            }
        }

        QString displayText = b.title.isEmpty() ? b.url : b.title;
        if (!b.folder.isEmpty() && currentFolder != b.folder) {
            displayText += QString(" [%1]").arg(b.folder);
        }
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, b.id);
        if (!b.favicon.isNull()) {
            item->setIcon(b.favicon);
        }
        m_bookmarkList->addItem(item);
    }
}

void BookmarkManager::populateFolders()
{
    m_folderCombo->clear();
    m_folderCombo->addItem("All");
    for (const QString& folder : m_folders) {
        m_folderCombo->addItem(folder);
    }
}

void BookmarkManager::clearForm()
{
    m_titleEdit->clear();
    m_urlEdit->clear();
    m_notesEdit->clear();
    m_selectedId.clear();
    m_addButton->setEnabled(true);
    //m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_launchButton->setEnabled(false);
}

void BookmarkManager::showBookmarkDetails(const Bookmark& bookmark)
{
    m_titleEdit->setText(bookmark.title);
    m_urlEdit->setText(bookmark.url);
    m_notesEdit->setPlainText(bookmark.notes);
    m_selectedId = bookmark.id;
    //m_addButton->setEnabled(false);
    m_editButton->setEnabled(true);
    m_deleteButton->setEnabled(true);
    m_launchButton->setEnabled(true);
}

bool BookmarkManager::loadBookmarks()
{
    QString filePath = getBookmarksFilePath();
    QFile file(filePath);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    in >> m_folders;

    quint32 count;
    in >> count;
    m_bookmarks.clear();
    for (quint32 i = 0; i < count; ++i) {
        Bookmark b;
        in >> b.id;
        in >> b.title;
        in >> b.url;
        in >> b.folder;
        in >> b.addedDate;
        in >> b.visitCount;
        in >> b.notes;
        m_bookmarks.append(b);
    }
    file.close();
    return true;
}

bool BookmarkManager::saveBookmarks()
{
    QString filePath = getBookmarksFilePath();
    QDir().mkpath(QFileInfo(filePath).dir().path());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    out << m_folders;

    out << quint32(m_bookmarks.size());
    for (const Bookmark& b : m_bookmarks) {
        out << b.id;
        out << b.title;
        out << b.url;
        out << b.folder;
        out << b.addedDate;
        out << b.visitCount;
        out << b.notes;
    }
    file.close();
    return true;
}

QString BookmarkManager::getBookmarksFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bookmarks.dat";
}

QPushButton *BookmarkManager::addButton() const
{
    return m_addButton;
}

void BookmarkManager::setCurrentUrl(const QString& url, const QString& title)
{
    m_currentUrl = url;
    m_currentTitle = title.isEmpty() ? url : title;
    clearForm();
    m_urlEdit->setText(m_currentUrl);
    m_titleEdit->setText(m_currentTitle);

}

void BookmarkManager::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);


}

void BookmarkManager::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}
