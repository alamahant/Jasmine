#include "searchpodcastdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include<QProgressDialog>
#include<QXmlStreamReader>


SearchPodcastDialog::SearchPodcastDialog(QWidget *parent)
    : QDialog(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_currentSelectedIndex(-1)
{
    setupUI();
    setWindowTitle("Search Podcasts");
    setMinimumSize(800, 600);
    resize(800, 600);
}

SearchPodcastDialog::~SearchPodcastDialog()
{
}

void SearchPodcastDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ========== SUBSCRIBE BY URL SECTION ==========
    QGroupBox *urlGroup = new QGroupBox("Subscribe by URL");
    QHBoxLayout *urlLayout = new QHBoxLayout(urlGroup);

    m_urlEdit = new QLineEdit();
    m_urlEdit->setPlaceholderText("https://example.com/feed.xml");
    m_urlSubscribeButton = new QPushButton("Subscribe");
    m_urlSubscribeButton->setObjectName("primaryButton");

    urlLayout->addWidget(m_urlEdit);
    urlLayout->addWidget(m_urlSubscribeButton);
    mainLayout->addWidget(urlGroup);

    // Add separator line
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    // Search section
    QGroupBox *searchGroup = new QGroupBox("Search");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchGroup);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Enter podcast name, category or author...");
    m_searchButton = new QPushButton("Search");
    m_searchButton->setObjectName("primaryButton");
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    mainLayout->addWidget(searchGroup);

    // Results section
    QGroupBox *resultsGroup = new QGroupBox("Results");
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);
    
    m_resultsTable = new QTableWidget();
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels({"Podcast", "Category", "Author"});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    resultsLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(resultsGroup);

    // Preview section
    QGroupBox *previewGroup = new QGroupBox("Preview");
    QFormLayout *previewLayout = new QFormLayout(previewGroup);
    
    m_previewName = new QLabel("-");
    m_previewName->setWordWrap(true);
    m_previewAuthor = new QLabel("-");
    m_previewCategory = new QLabel("-");
    m_previewFeedUrl = new QLabel("-");
    m_previewFeedUrl->setWordWrap(true);
    m_previewFeedUrl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    previewLayout->addRow("Name:", m_previewName);
    previewLayout->addRow("Author:", m_previewAuthor);
    previewLayout->addRow("Category:", m_previewCategory);
    previewLayout->addRow("Feed URL:", m_previewFeedUrl);
    
    mainLayout->addWidget(previewGroup);

    // Button section
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_subscribeButton = new QPushButton("Subscribe");
    m_subscribeButton->setObjectName("primaryButton");
    m_subscribeButton->setEnabled(false);
    
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("secondaryButton");
    
    buttonLayout->addWidget(m_subscribeButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_searchButton, &QPushButton::clicked, this, &SearchPodcastDialog::onSearchClicked);
    connect(m_resultsTable, &QTableWidget::itemSelectionChanged, this, &SearchPodcastDialog::onResultSelectionChanged);
    connect(m_subscribeButton, &QPushButton::clicked, this, &SearchPodcastDialog::onSubscribeClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    // Connect URL subscribe button
    connect(m_urlSubscribeButton, &QPushButton::clicked, this, &SearchPodcastDialog::onUrlSubscribeClicked);
}

void SearchPodcastDialog::performSearch()
{
    QString searchTerm = m_searchEdit->text().trimmed();
    if (searchTerm.isEmpty()) {
        QMessageBox::information(this, "Search", "Search by name, category or author");
        return;
    }
    
    // Clear previous results
    m_resultsTable->setRowCount(0);
    m_currentResults.clear();
    m_subscribeButton->setEnabled(false);
    clearPreview();
    
    // Build iTunes API URL
    QString url = QString("https://itunes.apple.com/search?term=%1&media=podcast&limit=100")
                      .arg(QUrl::toPercentEncoding(searchTerm));
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Jasmine/1.0");
    
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &SearchPodcastDialog::onSearchResult);
}

void SearchPodcastDialog::onSearchClicked()
{
    performSearch();
}

void SearchPodcastDialog::onSearchResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        onApiError(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QJsonArray results = obj["results"].toArray();
    
    m_resultsTable->setRowCount(results.size());
    
    for (int i = 0; i < results.size(); ++i) {
        QJsonObject item = results[i].toObject();
        PodcastShow show = parseJsonToPodcast(item);
        m_currentResults.append(show);
        
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(show.title));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(show.category));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(show.author));
    }
    
    m_resultsTable->resizeColumnsToContents();
    
    if (results.size() == 0) {
        QMessageBox::information(this, "Search Results", "No podcasts found matching your search.");
    }
    
    reply->deleteLater();
}

void SearchPodcastDialog::onResultSelectionChanged()
{
    int currentRow = m_resultsTable->currentRow();
    if (currentRow >= 0 && currentRow < m_currentResults.size()) {
        m_currentSelectedIndex = currentRow;
        updatePreview(m_currentResults[currentRow]);
        m_subscribeButton->setEnabled(true);
    } else {
        m_subscribeButton->setEnabled(false);
        clearPreview();
    }
}

void SearchPodcastDialog::onSubscribeClicked()
{
    if (m_currentSelectedIndex >= 0 && m_currentSelectedIndex < m_currentResults.size()) {
        emit podcastSelected(m_currentResults[m_currentSelectedIndex]);
        // Keep dialog open for multiple subscriptions
    }
}

void SearchPodcastDialog::onApiError(const QString &message)
{
    QMessageBox::warning(this, "API Error", "Failed to search: " + message);
}

PodcastShow SearchPodcastDialog::parseJsonToPodcast(const QJsonObject &obj)
{
    PodcastShow show;
    show.showId = QString::number(obj["collectionId"].toInt());
    show.title = obj["trackName"].toString();
    show.author = obj["artistName"].toString();
    show.artworkUrl = obj["artworkUrl100"].toString();
    show.feedUrl = obj["feedUrl"].toString();
    show.category = obj["primaryGenreName"].toString();
    show.websiteUrl = obj["collectionViewUrl"].toString();
    show.description = "";
    show.localArtworkPath = "";
    show.comments = "";
    show.episodeCount = 0;
    show.isSubscribed = false;
    show.lastUpdated = QDateTime();
    
    return show;
}

void SearchPodcastDialog::updatePreview(const PodcastShow &show)
{
    m_previewName->setText(show.title);
    m_previewAuthor->setText(show.author);
    m_previewCategory->setText(show.category);
    m_previewFeedUrl->setText(show.feedUrl);
}

void SearchPodcastDialog::clearPreview()
{
    m_previewName->setText("-");
    m_previewAuthor->setText("-");
    m_previewCategory->setText("-");
    m_previewFeedUrl->setText("-");
}

void SearchPodcastDialog::onUrlSubscribeClicked()
{
    QString feedUrl = m_urlEdit->text().trimmed();
    if (feedUrl.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a valid RSS feed URL.");
        return;
    }

    // Add http:// if missing
    if (!feedUrl.startsWith("http://") && !feedUrl.startsWith("https://")) {
        feedUrl = "http://" + feedUrl;
    }

    // Fetch and parse the feed
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(feedUrl)));

    QProgressDialog* progress = new QProgressDialog("Fetching podcast feed...", "Cancel", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(500);
    progress->show();

    connect(reply, &QNetworkReply::finished, [this, reply, nam, progress, feedUrl]() {
        progress->accept();
        if (reply->error() == QNetworkReply::NoError) {
            PodcastShow show = parseRSSFeedToShow(reply->readAll(), feedUrl);
            if (!show.title.isEmpty()) {
                emit podcastSelected(show);
                m_urlEdit->clear();
                QMessageBox::information(this, "Success", QString("Subscribed to: %1").arg(show.title));
            } else {
                QMessageBox::warning(this, "Error", "Invalid podcast feed. Could not parse title.");
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to fetch feed: " + reply->errorString());
        }
        reply->deleteLater();
        nam->deleteLater();
        progress->deleteLater();
    });

    connect(progress, &QProgressDialog::canceled, [reply, nam]() {
        reply->abort();
    });
}

PodcastShow SearchPodcastDialog::parseRSSFeedToShow(const QByteArray &data, const QString &feedUrl)
{
    PodcastShow show;
    show.feedUrl = feedUrl;
    show.isSubscribed = false;

    QXmlStreamReader xml(data);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                show.title = xml.readElementText();
            } else if (xml.name() == "description" || xml.name() == "summary") {
                show.description = xml.readElementText();
            } else if (xml.name() == "link") {
                show.websiteUrl = xml.readElementText();
            } else if (xml.name() == "image") {
                // Some feeds have image as child element
                while (!(xml.isEndElement() && xml.name() == "image")) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == "url") {
                        show.artworkUrl = xml.readElementText();
                    }
                }
            } else if (xml.name() == "url" && xml.attributes().value("rel").toString() == "image") {
                // Other feeds use link rel="image"
                show.artworkUrl = xml.attributes().value("href").toString();
            }
        }
    }

    // Generate a showId
    show.showId = QUuid::createUuid().toString();

    return show;
}
