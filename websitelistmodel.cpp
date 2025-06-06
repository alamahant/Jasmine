#include "websitelistmodel.h"
#include<QPixmap>

WebsiteListModel::WebsiteListModel(QObject *parent)
    : QAbstractListModel(parent) {}

int WebsiteListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_websites.size();
}

QVariant WebsiteListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_websites.size())
        return QVariant();

    const Website &website = m_websites.at(index.row());

    if (role == Qt::DisplayRole)
        return website.title;
    else if (role == Qt::DecorationRole)
        return website.favicon;  // Return favicon for decoration
    else if (role == Qt::UserRole)
        return QVariant::fromValue(website);  // Return the whole website object

    return QVariant();
}


void WebsiteListModel::addWebsite(const Website &website) {
    beginInsertRows(QModelIndex(), m_websites.size(), m_websites.size());
    m_websites.append(website);
    endInsertRows();
}

void WebsiteListModel::removeWebsite(int index) {
    if (index < 0 || index >= m_websites.size()) return;
    beginRemoveRows(QModelIndex(), index, index);
    m_websites.removeAt(index);
    endRemoveRows();
}

void WebsiteListModel::updateWebsite(int index, const Website &website) {
    if (index < 0 || index >= m_websites.size()) return;
    m_websites[index] = website;
    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
}

Website WebsiteListModel::getWebsite(int index) const {
    if (index < 0 || index >= m_websites.size()) return Website{};
    return m_websites.at(index);
}

QList<Website> WebsiteListModel::websites() const
{
    return m_websites;
}

void WebsiteListModel::setWebsites(const QList<Website>& websites) {
    beginResetModel();
    m_websites = websites;
    endResetModel();

    // Emit a signal to notify that the data has changed
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void WebsiteListModel::setWebsite(int row, const Website& website) {
    if (row >= 0 && row < m_websites.size()) {
        m_websites[row] = website;

        // Emit a signal to notify that the data has changed for this specific row
        QModelIndex modelIndex = index(row, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

QDataStream &operator<<(QDataStream &out, const Website &website) {
    out << website.url << website.title << website.username << website.password
        << website.comments << website.lastVisited << website.visitCount;
    QPixmap pixmap;
    if (!website.favicon.isNull()) {
        pixmap = website.favicon.pixmap(16, 16);
    }
    out << pixmap;
    return out;
}

QDataStream &operator>>(QDataStream &in, Website &website) {
    in >> website.url >> website.title >> website.username >> website.password
        >> website.comments >> website.lastVisited >> website.visitCount;
    QPixmap pixmap;
    in >> pixmap;
    if (!pixmap.isNull()) {
        website.favicon = QIcon(pixmap);
    }
    return in;
}


