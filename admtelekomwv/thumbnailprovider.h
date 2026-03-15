#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QQuickImageProvider>
#include <vlc/vlc.h>
#include <QImage>
#include <QDir>
#include <QEventLoop>

struct Context {
    uchar* buffer; // Массив для пикселей
    QEventLoop* loop;
    bool frameCaptured = false;
};

class ThumbnailProvider : public QQuickImageProvider
{
public:
    ThumbnailProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QImage getVlcThumbnail(QString videoPath, QString tempPath);
    static void* lock(void* data, void** planes);
    static void unlock(void* data, void*, void* const*);
};

#endif // THUMBNAILPROVIDER_H
