#pragma once

#include "VideoData.h"
#include <QList>
#include <QString>

class VideoDataLoader
{
public:
    static QList<VideoData> loadFromJsonFile(const QString &filePath);
    static QList<VideoData> loadFromResource(const QString &resourcePath);
};
