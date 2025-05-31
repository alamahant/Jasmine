#ifndef TWOFAMANAGER_H
#define TWOFAMANAGER_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QProgressBar>
#include <QMap>

struct TwoFAEntry {
    QString name;
    QString secret;
    QString issuer;
};

class TwoFAManager : public QWidget
{
    Q_OBJECT

public:
    explicit TwoFAManager(QWidget *parent = nullptr);
    ~TwoFAManager();

private slots:
    void onAddAccount();
    void onDeleteAccount();
    void onCopyCode();
    void onSelectionChanged();
    void updateCurrentCode();

private:
    void setupUI();
    void loadData();
    void saveData();
    void updateProgressBar();

    QListWidget* m_accountList;
    QLabel* m_currentCodeLabel;
    QLabel* m_timeRemainingLabel;
    QPushButton* m_deleteBtn;
    QPushButton* m_copyBtn;
    QTimer* m_updateTimer;
    QProgressBar* m_timeProgressBar;  // ← Add this member too

    QMap<QString, TwoFAEntry> m_entries;
};

#endif // TWOFAMANAGER_H

