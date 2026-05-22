#ifndef SEARCHIPTVDIALOG_H
#define SEARCHIPTVDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "iptvchannel.h"
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

class SearchIPTVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchIPTVDialog(QWidget *parent = nullptr);
    ~SearchIPTVDialog();

signals:
    void channelsSelected(const QVector<IPTVChannel> &channels);
    void previewChannel(const QString &streamUrl, const QString &name);
    void showNotification(int duration);
private slots:
    void onLoadRemoteM3U();
    void onBrowseLocalFile();
    void onLoadLocalM3U();
    void onFilterTextChanged(const QString &text);
    void onSelectAllClicked();
    void onAddSelectedClicked();
    void onPreviewClicked();
    void onXtreamConnect();  // Placeholder
    void onTableDoubleClicked(const QModelIndex &index);
private:
    void setupUI();
    void parseM3U(const QString &content, const QString &sourceName);
    void loadRemoteM3U(const QUrl &url);
    void loadLocalM3U(const QString &filePath);
    void displayChannels();
    void updatePreviewButtonState();
    
    // UI Components
    QTabWidget *m_tabWidget;
    
    // M3U Tab
    QLineEdit *m_remoteUrlEdit;
    QPushButton *m_loadRemoteButton;
    QLineEdit *m_localFileEdit;
    QPushButton *m_browseButton;
    QPushButton *m_loadLocalButton;
    
    // Xtream Tab (placeholder)
    QLineEdit *m_xtreamServerEdit;
    QLineEdit *m_xtreamUsernameEdit;
    QLineEdit *m_xtreamPasswordEdit;
    QLineEdit *m_xtreamUserAgentEdit;
    QPushButton *m_xtreamConnectButton;
    
    // Results section
    QLineEdit *m_filterEdit;
    QTableWidget *m_resultsTable;
    QPushButton *m_selectAllButton;
    QPushButton *m_previewButton;
    QPushButton *m_addButton;
    QPushButton *m_cancelButton;
    
    // Data
    QVector<IPTVChannel> m_allChannels;
    QVector<IPTVChannel> m_filteredChannels;
    QVector<bool> m_selectedRows;
    QString m_currentSource;
private:
    QFutureWatcher<QVector<IPTVChannel>> *m_parseWatcher = nullptr;
    void parseM3UAsync(const QString &content, const QString &sourceName);
    QPushButton* m_searchButton;
private slots:
    void onParseFinished();
    void onSearchClicked();


};

#endif // SEARCHIPTVDIALOG_H
