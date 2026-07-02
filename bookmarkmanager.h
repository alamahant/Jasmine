#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QDialog>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QUuid>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include<QCloseEvent>
#include<QShowEvent>

struct Bookmark {
    QString id;
    QString title;
    QString url;
    QString folder;
    QIcon favicon;
    QDateTime addedDate;
    int visitCount;
    QString notes;

    Bookmark() : visitCount(0) {}
    Bookmark(const QString& url, const QString& title)
        : id(QUuid::createUuid().toString())
        , title(title)
        , url(url)
        , addedDate(QDateTime::currentDateTime())
        , visitCount(0) {}
};

class BookmarkManager : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkManager(QWidget *parent = nullptr);
    ~BookmarkManager();

    // Data management
    void addBookmark(const Bookmark& bookmark);
    void removeBookmark(const QString& id);
    void updateBookmark(const Bookmark& bookmark);
    QList<Bookmark> getBookmarks() const;
    QList<Bookmark> getBookmarksInFolder(const QString& folder) const;
    QStringList getFolders() const;
    void addFolder(const QString& folderName);
    void removeFolder(const QString& folderName);
    void moveBookmarkToFolder(const QString& bookmarkId, const QString& folderName);
    QPushButton *addButton() const;

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void bookmarksChanged();
    void urlLaunched(const QString& url);

private slots:
    void onAddBookmark();
    void onEditBookmark();
    void onDeleteBookmark();
    void onFolderChanged(const QString& folder);
    void onBookmarkSelected();
    void onLaunchBookmark();
    void onAddFolder();
    void onDeleteFolder();
    void onClearForm();
    void onSearchTextChanged(const QString& text);

private:
    void populateBookmarks();
    void populateFolders();
    void clearForm();
    void showBookmarkDetails(const Bookmark& bookmark);
    void setupUI();

    bool loadBookmarks();
    bool saveBookmarks();
    QString getBookmarksFilePath() const;

    QListWidget* m_bookmarkList;
    QLineEdit* m_searchEdit;
    QLineEdit* m_titleEdit;
    QLineEdit* m_urlEdit;
    QComboBox* m_folderCombo;
    QTextEdit* m_notesEdit;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_launchButton;
    QPushButton* m_addFolderButton;
    QPushButton* m_deleteFolderButton;
    QString m_selectedId;

    QList<Bookmark> m_bookmarks;
    QStringList m_folders;

    QString m_currentUrl;
    QString m_currentTitle;
public slots:
    void setCurrentUrl(const QString& url, const QString& title);

};

#endif // BOOKMARKMANAGER_H
