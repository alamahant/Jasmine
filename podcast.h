#ifndef PODCAST_H
#define PODCAST_H

#include <QString>
#include <QDateTime>
#include <QVector>

struct PodcastEpisode {
    QString guid;
    QString title;
    QString description;
    QString audioUrl;
    QDateTime pubDate;
    int duration;  // seconds
    int playbackPosition;
    bool isPlayed = false;
    bool isDownloaded;
    QString localFilePath;
};

struct PodcastShow {
    QString showId;
    QString title;
    QString author;
    QString artworkUrl;
    QString feedUrl;
    QString websiteUrl;
    QString category;
    QString description;
    QString localArtworkPath;
    QString comments;
    QVector<PodcastEpisode> episodes;
    int episodeCount;
    bool isSubscribed;
    QDateTime lastUpdated;
};

#endif // PODCAST_H
