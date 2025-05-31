#ifndef WEBSITELISTMODEL_H
#define WEBSITELISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include<QDateTime>
#include <QDataStream>
#include<QIcon>

struct Website {
    QString url;
    QString title;
    QString username;
    QString password;
    QString comments;
    QDateTime lastVisited;
    int visitCount = 0;
    QIcon favicon;  // Store the favicon as a QIcon

    friend QDataStream &operator<<(QDataStream &out, const Website &website);
    friend QDataStream &operator>>(QDataStream &in, Website &website);
};

class WebsiteListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit WebsiteListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addWebsite(const Website &website);
    void removeWebsite(int index);
    void updateWebsite(int index, const Website &website);
    Website getWebsite(int index) const;

    QList<Website> websites() const;
    void setWebsites(const QList<Website>& websites);
    void setWebsite(int row, const Website& website);

    QList<Website> getWebsites() const {
        return m_websites;
    }

private:
    QList<Website> m_websites;
};

#endif // WEBSITELISTMODEL_H
