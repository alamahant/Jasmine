#include "searchradiostationsdialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QStatusBar>
#include<QFormLayout>
#include<QGroupBox>
#include<QFile>
#include "Constants.h"
#include"countrymapper.h"

SearchRadioStationsDialog::SearchRadioStationsDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentSelectedIndex(-1)

{

    setupUI();
    populateCountryCombo();
    
    m_api = new RadioBrowserAPI(this);
    connect(m_api, &RadioBrowserAPI::stationsFound, this, &SearchRadioStationsDialog::onSearchResult);
    connect(m_api, &RadioBrowserAPI::error, this, &SearchRadioStationsDialog::onApiError);
    
    setWindowTitle("Search Radio Stations");
    setMinimumSize(800, 600);
}

SearchRadioStationsDialog::~SearchRadioStationsDialog()
{
}


void SearchRadioStationsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    
    QGroupBox *searchGroup = new QGroupBox("Search Criteria");
    QFormLayout *searchLayout = new QFormLayout(searchGroup);
    
    m_countryCombo = new QComboBox();
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("Enter station name (optional)");
    
    searchLayout->addRow("Country:", m_countryCombo);
    searchLayout->addRow("Station Name:", m_nameEdit);
    
    m_searchButton = new QPushButton("Search");
    m_searchButton->setObjectName("primaryButton");
    searchLayout->addRow("", m_searchButton);
    
    mainLayout->addWidget(searchGroup);
    
    QGroupBox *resultsGroup = new QGroupBox("Results");
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);
    


    m_resultsTable = new QTableWidget();
    m_resultsTable->setColumnCount(5);
    m_resultsTable->setHorizontalHeaderLabels({"Station Name", "Country", "Bitrate", "Codec", "Genre"});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setAlternatingRowColors(true);
    
    resultsLayout->addWidget(m_resultsTable);
    mainLayout->addWidget(resultsGroup);
    

    QGroupBox *filterGroup = new QGroupBox("Filter Results");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);

    QLabel *filterLabel = new QLabel("Filter:");
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Type to filter current results...");
    connect(m_filterEdit, &QLineEdit::textChanged, this, &SearchRadioStationsDialog::filterResults);

    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterEdit);

    mainLayout->insertWidget(mainLayout->indexOf(resultsGroup) + 1, filterGroup);

    QGroupBox *previewGroup = new QGroupBox("Preview");
    QFormLayout *previewLayout = new QFormLayout(previewGroup);
    
    m_previewName = new QLabel("-");
    m_previewUrl = new QLabel("-");
    m_previewUrl->setWordWrap(true);
    m_previewCountry = new QLabel("-");
    m_previewBitrate = new QLabel("-");
    m_previewCodec = new QLabel("-");
    m_previewGenre = new QLabel("-");
    
    previewLayout->addRow("Name:", m_previewName);
    previewLayout->addRow("Stream URL:", m_previewUrl);
    previewLayout->addRow("Country:", m_previewCountry);
    previewLayout->addRow("Bitrate:", m_previewBitrate);
    previewLayout->addRow("Codec:", m_previewCodec);
    previewLayout->addRow("Genre:", m_previewGenre);
    
    mainLayout->addWidget(previewGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    //
    QPushButton *infoButton = new QPushButton("i", this);
    infoButton->setFixedSize(32, 32);
    infoButton->setObjectName("infoButton");
    //infoButton->setEnabled(false);
    infoButton->setToolTip(
        "<html><body>"
        "<b>Search Radio Stations — Quick Guide</b><br><br>"

        "<b>Searching:</b><br>"
        "• Pick a <b>Country</b> from the dropdown (default: All)<br>"
        "• Optionally type a <b>Station Name</b><br>"
        "• Click <b>Search</b> to query the Radio Browser database<br><br>"

        "<b>Results Table:</b><br>"
        "• Shows Name, Country, Bitrate, Codec, Genre<br>"
        "• Click any row to load it into the Preview panel<br>"
        "• Higher bitrate usually means better audio quality<br><br>"

        "<b>Filter:</b><br>"
        "• Type in the <b>Filter</b> box to narrow visible rows<br>"
        "• Matches on Station Name and Genre<br><br>"

        "<b>Actions:</b><br>"
        "• <b>Preview</b> plays the station without adding it<br>"
        "• <b>Add to My Stations</b> saves the station to your list<br>"
        "• <b>Cancel</b> closes without changes<br><br>"

        "<b>Notes:</b><br>"
        "• Results come from radio-browser.info, a community database<br>"
        "• Some stations may be offline or geo-blocked<br>"
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
    //
    buttonLayout->addWidget(infoButton);
    buttonLayout->addStretch();
    
    m_playButton = new QPushButton("Preview");
    m_playButton->setObjectName("secondaryButton");
    m_playButton->setEnabled(false);
    


    m_addButton = new QPushButton("Add to My Stations");
    m_addButton->setObjectName("primaryButton");
    m_addButton->setEnabled(false);
    
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("secondaryButton");
    
    buttonLayout->addWidget(m_playButton);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(m_searchButton, &QPushButton::clicked, this, &SearchRadioStationsDialog::onSearchClicked);
    connect(m_resultsTable, &QTableWidget::itemSelectionChanged, this, &SearchRadioStationsDialog::onResultSelectionChanged);
    connect(m_playButton, &QPushButton::clicked, this, &SearchRadioStationsDialog::onPlayClicked);
    connect(m_addButton, &QPushButton::clicked, this, &SearchRadioStationsDialog::onAddClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SearchRadioStationsDialog::populateCountryCombo()
{
    m_countryCombo->addItem("All", "");
    
    QList<QString> countryCodes = CountryMapper::getAllCountryCodes();
    for (const QString &code : countryCodes) {
        QString countryName = CountryMapper::getCountryNameFromCode(code);
        m_countryCombo->addItem(countryName, code);
    }
}

void SearchRadioStationsDialog::performSearch()
{
    QString countryCode = m_countryCombo->currentData().toString();
    QString stationName = m_nameEdit->text().trimmed();
    
    m_resultsTable->setRowCount(0);
    m_currentResults.clear();
    m_playButton->setEnabled(false);
    m_addButton->setEnabled(false);
    clearPreview();
    
    if (m_api) {
        if (!stationName.isEmpty()) {
            if (!countryCode.isEmpty()) {
                m_api->searchStations(stationName, countryCode);
            } else {
                m_api->searchStations(stationName);
            }
        } else if (!countryCode.isEmpty()) {
            m_api->searchStationsByCountry(countryCode);
        } else {
            QMessageBox::information(this, "Search", "Please enter a station name or select a country.");
        }
    }
}

void SearchRadioStationsDialog::onSearchClicked()
{
    performSearch();
}

void SearchRadioStationsDialog::onSearchResult(const QJsonArray &stations)
{
    m_currentResults.clear();
    m_resultsTable->setRowCount(stations.size());
    
    for (int i = 0; i < stations.size(); ++i) {
        QJsonObject obj = stations[i].toObject();
        RadioStation station = parseJsonToStation(obj);
        m_currentResults.append(station);
        
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(station.name));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(CountryMapper::getCountryNameFromCode(station.countrycode)));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(station.bitrate) + " kbps"));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(station.codec));
        m_resultsTable->setItem(i, 4, new QTableWidgetItem(station.genre));
    }
    
    m_resultsTable->resizeColumnsToContents();
    
    if (stations.size() == 0) {
        QMessageBox::information(this, "Search Results", "No stations found matching your criteria.");
    }
}

void SearchRadioStationsDialog::onResultSelectionChanged()
{
    int currentRow = m_resultsTable->currentRow();
    if (currentRow >= 0 && currentRow < m_currentResults.size()) {
        m_currentSelectedIndex = currentRow;
        updatePreview(m_currentResults[currentRow]);
        m_playButton->setEnabled(true);
        m_addButton->setEnabled(true);
    } else {
        m_playButton->setEnabled(false);
        m_addButton->setEnabled(false);
        clearPreview();
    }
}

void SearchRadioStationsDialog::onPlayClicked()
{
    if (m_currentSelectedIndex >= 0 && m_currentSelectedIndex < m_currentResults.size()) {
        const RadioStation &station = m_currentResults[m_currentSelectedIndex];
        if (!station.streamUrl.isEmpty()) {
            emit showNotification(QString(), 4000);

            emit previewRadioStation(station.streamUrl, station.name);
        }
    }
}

void SearchRadioStationsDialog::onAddClicked()
{
    if (m_currentSelectedIndex >= 0 && m_currentSelectedIndex < m_currentResults.size()) {

        RadioStation station = m_currentResults[m_currentSelectedIndex];
        emit showNotification(QString("Added %1").arg(station.name), 3000);

        if (!station.iconUrl.isEmpty()) {

            QNetworkAccessManager *nam = new QNetworkAccessManager(this);
            QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(station.iconUrl)));

            connect(reply, &QNetworkReply::finished, [this, reply, nam, station]() mutable {
                if (reply->error() == QNetworkReply::NoError) {
                    QString iconDir = JASMINE_CONSTANTS::iconDir;

                    QString iconPath = iconDir + "/" + station.stationuuid + ".png";
                    QFile file(iconPath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(reply->readAll());
                        file.close();

                        RadioStation stationWithIcon = station;
                        stationWithIcon.iconUrl = iconPath;  // Store local path instead of URL

                        emit stationSelected(stationWithIcon);
                    } else {
                        emit stationSelected(station);  // Fallback: no icon
                    }
                } else {
                    emit stationSelected(station);  // Fallback: no icon
                }
                reply->deleteLater();
                nam->deleteLater();
            });
        } else {
            emit stationSelected(station);
        }


    }
}

void SearchRadioStationsDialog::onApiError(const QString &message)
{
    QMessageBox::warning(this, "API Error", "Failed to search: " + message);
}

RadioStation SearchRadioStationsDialog::parseJsonToStation(const QJsonObject &obj)
{
    RadioStation station;
    station.stationuuid = obj["stationuuid"].toString();
    station.name = obj["name"].toString();
    station.streamUrl = obj["url_resolved"].toString();
    if (station.streamUrl.isEmpty()) {
        station.streamUrl = obj["url"].toString();
    }
    station.iconUrl = obj["favicon"].toString();
    station.countrycode = obj["countrycode"].toString();
    station.genre = obj["tags"].toString();
    station.bitrate = obj["bitrate"].toInt();
    station.codec = obj["codec"].toString();
    station.votes = obj["votes"].toInt();
    station.comments = "";
    station.isPlaying = false;
    
    return station;
}

void SearchRadioStationsDialog::updatePreview(const RadioStation &station)
{
    m_previewName->setText(station.name);
    m_previewUrl->setText(station.streamUrl);
    m_previewCountry->setText(CountryMapper::getCountryNameFromCode(station.countrycode));
    m_previewBitrate->setText(QString::number(station.bitrate) + " kbps");
    m_previewCodec->setText(station.codec);
    m_previewGenre->setText(station.genre);
}

void SearchRadioStationsDialog::clearPreview()
{
    m_previewName->setText("-");
    m_previewUrl->setText("-");
    m_previewCountry->setText("-");
    m_previewBitrate->setText("-");
    m_previewCodec->setText("-");
    m_previewGenre->setText("-");
}

RadioBrowserAPI *SearchRadioStationsDialog::api() const
{
    return m_api;
}

void SearchRadioStationsDialog::filterResults(const QString &text)
{
    if (text.isEmpty()) {
        for (int i = 0; i < m_resultsTable->rowCount(); ++i) {
            m_resultsTable->showRow(i);
        }
    } else {
        for (int i = 0; i < m_resultsTable->rowCount(); ++i) {
            QString stationName = m_resultsTable->item(i, 0)->text();
            QString genre = m_resultsTable->item(i, 4)->text();

            bool matches = stationName.contains(text, Qt::CaseInsensitive) ||
                           genre.contains(text, Qt::CaseInsensitive);

            m_resultsTable->setRowHidden(i, !matches);
        }
    }
}

