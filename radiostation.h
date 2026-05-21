#pragma once
#include<QString>

// Radio data structures
        struct RadioStation {
            QString stationuuid;
            QString name;
            QString streamUrl;
            QString iconUrl;
            QString countrycode;
            QString genre;
            int bitrate;
            QString codec;
            int votes;
            QString comments;
            bool isPlaying;
        };
