#pragma once

#include <QString>

struct VideoData
{
    QString title;
    QString coverPath;         // URL or local path for cover image
    QString uploaderName;
    QString uploaderAvatarPath;
    QString playCount;         // formatted: "12.3万"
    QString publishTime;       // formatted: "3天前"
    QString duration;          // formatted: "12:34"
    QString bvid;              // BV video ID for constructing links
};
