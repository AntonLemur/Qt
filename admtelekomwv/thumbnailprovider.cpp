#include "thumbnailprovider.h"
#include <QThread>
#include <QTimer>

ThumbnailProvider::ThumbnailProvider(): QQuickImageProvider(QQuickImageProvider::Image) {}

QImage ThumbnailProvider::getVlcThumbnail(QString videoPath, QString tempPath)
{
    const int width = 640;  // Фиксируем размер превью для скорости
    const int height = 360;

    const char *vlc_args[] = { "--no-audio", "--no-video-title-show", "--avcodec-hw=none" };
    libvlc_instance_t *vlc = libvlc_new(3, vlc_args);
    libvlc_media_t *m = libvlc_media_new_location(vlc, videoPath.toUtf8().constData());
    libvlc_media_player_t *mp = libvlc_media_player_new_from_media(m);

    // Подготовка буфера (RGBA: 4 байта на пиксель)
    QByteArray buffer(width * height * 4, 0);
    QEventLoop loop;
    Context ctx { (uchar*)buffer.data(), &loop, false };

    // Настраиваем VLC на рисование в память (формат RV32 = RGBA)
    libvlc_video_set_format(mp, "RV32", width, height, width * 4);
    libvlc_video_set_callbacks(mp, &ThumbnailProvider::lock, &ThumbnailProvider::unlock, nullptr, &ctx);

    libvlc_media_player_play(mp);

    // Ждем кадра 15 секунд (для сети)
    QTimer::singleShot(15000, &loop, &QEventLoop::quit);
    loop.exec();

    libvlc_media_player_stop(mp);

    QImage result;
    if (ctx.frameCaptured) {
        // Копируем данные из буфера в QImage
        result = QImage((uchar*)buffer.data(), width, height, QImage::Format_RGB32).copy();
    }

    libvlc_media_player_release(mp);
    libvlc_media_release(m);
    libvlc_release(vlc);

    return result;
}

QImage ThumbnailProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString videoPath = id; // Например, "C:/Videos/movie.mp4"
    QString tempImg = QDir::tempPath() + "/thumb_" + QString::number(qHash(videoPath)) + ".png";

    qDebug() << "videoPath: " << videoPath << ", tempImg: " << tempImg;

    QImage thumb = getVlcThumbnail(videoPath, tempImg);

    if (size) *size = thumb.size();

    // Удаляем временный файл после загрузки в память
    QFile::remove(tempImg);

    return thumb;
}

// Callback: VLC спрашивает, куда рисовать
void* ThumbnailProvider::lock(void* data, void** planes) {
    Context* ctx = static_cast<Context*>(data);
    *planes = ctx->buffer; // Отдаем адрес нашего массива
    return nullptr;
}

// Callback: Кадр отрисован в память
void ThumbnailProvider::unlock(void* data, void*, void* const*) {
    Context* ctx = static_cast<Context*>(data);
    if (!ctx->frameCaptured) {
        ctx->frameCaptured = true;
        ctx->loop->quit(); // Выходим из ожидания, кадр у нас!
    }
}
