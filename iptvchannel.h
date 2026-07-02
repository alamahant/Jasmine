#ifndef IPTVCHANNEL_H
#define IPTVCHANNEL_H

#include <QString>

struct IPTVChannel {
    QString channelId;      // tvg-id from M3U
    QString name;           // channel name
    QString streamUrl;      // .m3u8 URL
    QString logoUrl;        // tvg-logo
    
    QString category;       // from group-title
    QString country;        // user can set/edit
    QString resolution;  // e.g., "720p", "1080p", "576i", "4K"
    QString referrer;       // http-referrer
    QString userAgent;      // http-user-agent
    
    QString comments;
    bool isPlaying;
    QString localLogoPath;

};

#endif // IPTVCHANNEL_H
