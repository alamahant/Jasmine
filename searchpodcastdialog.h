#ifndef SEARCHPODCASTDIALOG_H
#define SEARCHPODCASTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "podcast.h"

class SearchPodcastDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchPodcastDialog(QWidget *parent = nullptr);
    ~SearchPodcastDialog();

signals:
    void podcastSelected(const PodcastShow &show);

private slots:
    void onSearchClicked();
    void onSearchResult();
    void onResultSelectionChanged();
    void onSubscribeClicked();
    void onApiError(const QString &message);
    void onUrlSubscribeClicked();
private:
    void setupUI();
    void performSearch();
    void clearPreview();
    void updatePreview(const PodcastShow &show);
    PodcastShow parseJsonToPodcast(const QJsonObject &obj);

    // UI Components
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QTableWidget *m_resultsTable;
    
    // Preview section
    QLabel *m_previewName;
    QLabel *m_previewAuthor;
    QLabel *m_previewCategory;
    QLabel *m_previewFeedUrl;
    
    // Buttons
    QPushButton *m_subscribeButton;
    QPushButton *m_cancelButton;
    
    // API and data
    QNetworkAccessManager *m_nam;
    QVector<PodcastShow> m_currentResults;
    int m_currentSelectedIndex;
    QLineEdit* m_urlEdit;
    QPushButton* m_urlSubscribeButton;
    PodcastShow parseRSSFeedToShow(const QByteArray &data, const QString &feedUrl);

};

#endif // SEARCHPODCASTDIALOG_H
