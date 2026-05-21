#ifndef RADIOBROWSERAPI_H
#define RADIOBROWSERAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QRandomGenerator>

class RadioBrowserAPI : public QObject
{
    Q_OBJECT

public:
    explicit RadioBrowserAPI(QObject *parent = nullptr);
    ~RadioBrowserAPI();

    void searchStations(const QString &searchTerm);
    void searchStations(const QString &searchTerm, const QString &countryCode);
    void searchStationsByCountry(const QString &countryCode);
    void refreshServers();

signals:
    void stationsFound(const QJsonArray &stations);
    void error(const QString &message);

private slots:
    void handleReply(QNetworkReply *reply);

private:
    QString getRandomServer();
    void performRequest(const QUrl &url);

    QNetworkAccessManager *m_nam;
    QStringList m_servers;
    void discoverServers();

};

#endif // RADIOBROWSERAPI_H
