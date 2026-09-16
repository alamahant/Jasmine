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

    QGroupBox *urlGroup = new QGroupBox("Subscribe by URL");
    QHBoxLayout *urlLayout = new QHBoxLayout(urlGroup);

    m_urlEdit = new QLineEdit();
    m_urlEdit->setPlaceholderText("https://example.com/feed.xml");
    m_urlSubscribeButton = new QPushButton("Subscribe");
    m_urlSubscribeButton->setObjectName("primaryButton");

    urlLayout->addWidget(m_urlEdit);
    urlLayout->addWidget(m_urlSubscribeButton);
    mainLayout->addWidget(urlGroup);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    QGroupBox *searchGroup = new QGroupBox("Search");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchGroup);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Enter podcast name, category or author...");
    m_searchButton = new QPushButton("Search");
    m_searchButton->setObjectName("primaryButton");
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    mainLayout->addWidget(searchGroup);

    QGroupBox *resultsGroup = new QGroupBox("Results");
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);
    
    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels({"Podcast", "Category", "Author"});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);

#ifndef FLATPAK_BUILD
    m_resultsTable->setAlternatingRowColors(false);

#else
    m_resultsTable->setAlternatingRowColors(true);

#endif
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    resultsLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(resultsGroup);

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

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    //
    QPushButton *infoButton = new QPushButton("i", this);
    infoButton->setFixedSize(32, 32);
    infoButton->setObjectName("infoButton");
    infoButton->setToolTip(
        "<html><body>"
        "<b>Search Podcasts — Quick Guide</b><br><br>"

        "<b>Subscribe by URL:</b><br>"
        "• Paste a podcast RSS feed URL and click <b>Subscribe</b><br>"
        "• Works with any valid RSS/Atom podcast feed<br>"
        "• The app fetches and parses the feed to extract title, artwork, and description<br><br>"

        "<b>Search:</b><br>"
        "• Type a podcast name, category, or author<br>"
        "• Click <b>Search</b> to query the iTunes podcast directory<br>"
        "• Up to 100 results are returned<br><br>"

        "<b>Results Table:</b><br>"
        "• Shows Podcast name, Category, Author<br>"
        "• Click a row to load it into the Preview panel<br>"
        "• Click <b>Subscribe</b> to add the selected podcast<br><br>"

        "<b>Preview Panel:</b><br>"
        "• Displays the selected podcast's details<br>"
        "• Feed URL is selectable so you can copy it<br><br>"

        "<b>Actions:</b><br>"
        "• <b>Subscribe</b> adds the selected podcast to your library<br>"
        "• <b>Cancel</b> closes without changes<br><br>"

        "<b>Notes:</b><br>"
        "• Search uses the public iTunes Search API — no account needed<br>"
        "• Some feeds may be private or geo-restricted<br>"
        "• If a feed fails to parse, verify the URL points to a valid RSS file<br>"
        "</body></html>"
    );
    infoButton->setStyleSheet(
        "QPushButton#infoButton { "
        "background-color: rgba(37, 99, 235, 0.15); "
        "font-size: 18px; "
        "font-weight: bold; "
        "color: #60a5fa; "
        "border: none; "
        "padding: 0px; "
        "} "
        "QPushButton#infoButton:hover { "
        "background-color: rgba(37, 99, 235, 0.3); "
        "color: #93bbfc; "
        "}"
    );
    buttonLayout->addWidget(infoButton);
    //
    buttonLayout->addStretch();
    
    m_subscribeButton = new QPushButton("Subscribe");
    m_subscribeButton->setObjectName("primaryButton");
    m_subscribeButton->setEnabled(false);
    
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("secondaryButton");
    
    buttonLayout->addWidget(m_subscribeButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);

    connect(m_searchButton, &QPushButton::clicked, this, &SearchPodcastDialog::onSearchClicked);
    connect(m_resultsTable, &QTableWidget::itemSelectionChanged, this, &SearchPodcastDialog::onResultSelectionChanged);
    connect(m_subscribeButton, &QPushButton::clicked, this, &SearchPodcastDialog::onSubscribeClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_urlSubscribeButton, &QPushButton::clicked, this, &SearchPodcastDialog::onUrlSubscribeClicked);
}

void SearchPodcastDialog::performSearch()
{
    QString searchTerm = m_searchEdit->text().trimmed();
    if (searchTerm.isEmpty()) {
        QMessageBox::information(this, "Search", "Search by name, category or author");
        return;
    }
    
    m_resultsTable->setRowCount(0);
    m_currentResults.clear();
    m_subscribeButton->setEnabled(false);
    clearPreview();
    
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
        emit showNotification(QString("%1 Added!").arg(m_currentResults[m_currentSelectedIndex].title), 7000);

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

    if (!feedUrl.startsWith("http://") && !feedUrl.startsWith("https://")) {
        feedUrl = "http://" + feedUrl;
    }


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
                while (!(xml.isEndElement() && xml.name() == "image")) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == "url") {
                        show.artworkUrl = xml.readElementText();
                    }
                }
            } else if (xml.name() == "url" && xml.attributes().value("rel").toString() == "image") {
                show.artworkUrl = xml.attributes().value("href").toString();
            }
        }
    }

    show.showId = QUuid::createUuid().toString();

    return show;
}
