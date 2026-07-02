#pragma once
#include<QString>

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
