#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QDialog>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QListWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include<QCloseEvent>

struct HistoryEntry {
    QString url;
    QString title;
    QIcon favicon;
    QDateTime visitTime;
    int visitCount;
    
    HistoryEntry() : visitCount(0) {}
    HistoryEntry(const QString& url, const QString& title)
        : url(url), title(title), visitTime(QDateTime::currentDateTime()), visitCount(1) {}
};

class HistoryManager : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryManager(QWidget *parent = nullptr);
    ~HistoryManager();

    // Data management
    void addEntry(const QString& url, const QString& title, const QIcon& favicon = QIcon());
    void clearHistory();
    void removeEntry(const QString& url);
    QList<HistoryEntry> getHistory() const;
    QList<HistoryEntry> getTodayHistory() const;
    QList<HistoryEntry> getHistoryForDate(const QDate& date) const;
    QList<HistoryEntry> searchHistory(const QString& query) const;
protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void historyChanged();
    void urlLaunched(const QString& url);

private slots:
    void onClearHistory();
    void onDeleteSelected();
    void onLaunchSelected();
    void onFilterChanged();
    void onSearchTextChanged(const QString& text);
    void showContextMenu(const QPoint& pos);

private:
    void populateHistory();
    void setupUI();
    
    bool loadHistory();
    bool saveHistory();
    QString getHistoryFilePath() const;

    QListWidget* m_historyList;
    QLineEdit* m_searchEdit;
    QDateEdit* m_dateFilter;
    QPushButton* m_clearButton;
    QPushButton* m_deleteButton;
    QPushButton* m_launchButton;
    
    QList<HistoryEntry> m_history;
    static const int MAX_HISTORY_ENTRIES = 1000;
};

#endif // HISTORYMANAGER_H
