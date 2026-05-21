#include "radiobrowserapi.h"
#include <QUrl>
#include <QUrlQuery>
#include<QSettings>

RadioBrowserAPI::RadioBrowserAPI(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    QSettings settings;
    QStringList savedServers = settings.value("radioBrowser/servers").toStringList();

    if (!savedServers.isEmpty()) {
        m_servers = savedServers;
    } else {
        // First run - discover servers
        discoverServers();
    }
}



RadioBrowserAPI::~RadioBrowserAPI()
{
}

QString RadioBrowserAPI::getRandomServer()
{
    if (m_servers.isEmpty())
        return QString();
    
    int randomIndex = QRandomGenerator::global()->bounded(m_servers.size());
    return m_servers.at(randomIndex);
}

void RadioBrowserAPI::performRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Jasmine/1.0");
    
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void RadioBrowserAPI::searchStations(const QString &searchTerm)
{
    QString server = getRandomServer();
    if (server.isEmpty()) {
        emit error("No servers available");
        return;
    }
    
    QUrl url(server + "/json/stations/search");
    QUrlQuery query;
    query.addQueryItem("name", searchTerm);
    //query.addQueryItem("limit", "50");
    query.addQueryItem("hidebroken", "true");
    url.setQuery(query);
    
    performRequest(url);
}

void RadioBrowserAPI::searchStations(const QString &searchTerm, const QString &countryCode)
{
    QString server = getRandomServer();
    if (server.isEmpty()) {
        emit error("No servers available");
        return;
    }
    
    QUrl url(server + "/json/stations/search");
    QUrlQuery query;
    query.addQueryItem("name", searchTerm);
    query.addQueryItem("countrycode", countryCode);
    //query.addQueryItem("limit", "50");
    query.addQueryItem("hidebroken", "true");
    url.setQuery(query);
    
    performRequest(url);
}

void RadioBrowserAPI::searchStationsByCountry(const QString &countryCode)
{
    QString server = getRandomServer();
    if (server.isEmpty()) {
        emit error("No servers available");
        return;
    }
    
    QUrl url(server + "/json/stations/search");
    QUrlQuery query;
    query.addQueryItem("countrycode", countryCode);
    query.addQueryItem("order", "clickcount");   // ← Add this for popularity
    query.addQueryItem("reverse", "true");       // ← Highest first
    //query.addQueryItem("limit", "50");
    query.addQueryItem("hidebroken", "true");
    url.setQuery(query);
    
    performRequest(url);
}

void RadioBrowserAPI::handleReply(QNetworkReply *reply)
{
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        emit error("Network error: " + reply->errorString());
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    
    if (!doc.isArray()) {
        emit error("Invalid response format");
        return;
    }
    
    emit stationsFound(doc.array());
}

void RadioBrowserAPI::discoverServers()
{
    QNetworkRequest request(QUrl("https://all.api.radio-browser.info/json/servers"));
    QNetworkReply *reply = m_nam->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isArray()) {
                m_servers.clear();
                QSet<QString> uniqueNames;
                QJsonArray servers = doc.array();
                for (const QJsonValue &val : servers) {
                    QJsonObject obj = val.toObject();
                    QString name = obj["name"].toString();
                    if (!name.isEmpty() && !uniqueNames.contains(name)) {
                        uniqueNames.insert(name);
                        m_servers.append("https://" + name);
                    }
                }
                // Save to QSettings
                QSettings settings;
                settings.setValue("radioBrowser/servers", m_servers);
            }
        }

        // Fallback if discovery failed
        if (m_servers.isEmpty()) {
            m_servers = {
                "https://de1.api.radio-browser.info",
                "https://de2.api.radio-browser.info",
                "https://fr1.api.radio-browser.info",
                "https://nl1.api.radio-browser.info"
            };
        }

        reply->deleteLater();
    });
}

void RadioBrowserAPI::refreshServers()
{
    discoverServers();
}
