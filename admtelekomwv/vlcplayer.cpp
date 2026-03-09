#include "vlcplayer.h"
#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_events.h>
#include <QPainter>
#include <QVideoFrame>
#include <QDebug>

VLCPlayer::VLCPlayer(): m_format(QSize(VIDEO_WIDTH, VIDEO_HEIGHT), QVideoFrameFormat::Format_RGBA8888) {
    videoBuffer.resize(VIDEO_WIDTH * VIDEO_HEIGHT * 4); // RGBA
    // const char * const vlc_args[] = {
    //     "--network-caching=300",       // Базовый кэш сети (300 мс)
    //     "--clock-jitter=0",            // Не ждать пакеты, если они опаздывают
    //     "--clock-synchro=0",           // Отключить строгую синхронизацию A/V
    //     "--drop-late-frames",          // КРИТИЧНО: выкидывать кадры, которые опоздали
    //     "--skip-frames"                // Пропускать декодирование, если не успеваем
    // };

    // const char * const vlc_args[] = {
    //     "--clock-jitter=0",
    //     "--clock-synchro=0",
    //     "--network-caching=100",  // Очень агрессивно
    //     "--sout-mux-caching=100",
    //     "--no-audio",             // Временно отключите звук, чтобы проверить, не он ли тормозит поток
    // };

    _vlcInstance = libvlc_new(0, nullptr);
    _vlcPlayer = libvlc_media_player_new(_vlcInstance);

    // Set the format: RGBA (4 bytes per pixel), width, height, pitch
    libvlc_video_set_format(_vlcPlayer, "RGBA", VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);

    // Link our functions
    libvlc_video_set_callbacks(_vlcPlayer, &VLCPlayer::lock, &VLCPlayer::unlock, &VLCPlayer::display, this);

    // Подписываемся на события VLC
    libvlc_event_manager_t *em = libvlc_media_player_event_manager(_vlcPlayer);
    libvlc_event_attach(em, libvlc_MediaPlayerTimeChanged, &VLCPlayer::vlc_callback, this);
    libvlc_event_attach(em, libvlc_MediaPlayerSeekableChanged, &VLCPlayer::vlc_callback, this);
    libvlc_event_attach(em, libvlc_MediaPlayerLengthChanged, &VLCPlayer::vlc_callback, this);
}

void VLCPlayer::setSource(const QString &path) {
    libvlc_media_t *m = libvlc_media_new_location(_vlcInstance, path.toUtf8().constData());

    // Используем асинхронный парсинг старого образца
    //libvlc_media_parse_async(m);

    //Для rtsp
    libvlc_media_add_option(m, ":codec=mediacodec,none"); // Использовать MediaCodec Android

    // 1. Форсируем UDP (TCP дает задержки при потере пакетов)
    libvlc_media_add_option(m, ":rtsp-tcp=0");

    // 2. Минимальный сетевой кэш
    libvlc_media_add_option(m, ":network-caching=150");

    // 3. Отключаем джиттер-буфер (сглаживание дрожания сети)
    libvlc_media_add_option(m, ":clock-jitter=0");

    // 4. Опция для моментального вывода кадра
    libvlc_media_add_option(m, ":rtsp-frame-buffer-size=0");

    libvlc_media_player_set_media(_vlcPlayer, m);
    libvlc_media_release(m);
    // libvlc_media_player_play(_vlcPlayer);
}

QVideoSink* VLCPlayer::videoSink() const
{
    return m_videoSink.get();
}

void VLCPlayer::setVideoSink(QVideoSink *newVideoSink)
{
    if (m_videoSink == newVideoSink)
        return;
    m_videoSink = newVideoSink;
    emit videoSinkChanged();
}

void* VLCPlayer::lock(void *opaque, void **planes) {
    VLCPlayer *player = static_cast<VLCPlayer*>(opaque);

    // 1. Создаем новый кадр в памяти видео-движка Qt
    player->m_currentFrame = QVideoFrame(player->m_format);

    // 2. "Открываем" память кадра для записи
    if (player->m_currentFrame.map(QVideoFrame::WriteOnly)) {
        // 3. Передаем указатель на память кадра в VLC
        planes[0] = player->m_currentFrame.bits(0);
    }

    return nullptr;
}

void VLCPlayer::unlock(void *opaque, void *picture, void *const *planes) {
    VLCPlayer *player = static_cast<VLCPlayer*>(opaque);

    // Закрываем доступ к памяти (теперь кадр готов к отображению)
    player->m_currentFrame.unmap();
}

void VLCPlayer::display(void *opaque, void *picture) {
    VLCPlayer *player = static_cast<VLCPlayer*>(opaque);

    // Передаем готовый объект кадра в основной поток через QueuedConnection
    // Qt 6 очень эффективно передает QVideoFrame (используется счетчик ссылок)
    QMetaObject::invokeMethod(player, [player](){
        if (player->m_videoSink) {
            player->m_videoSink->setVideoFrame(player->m_currentFrame);
        }
    }, Qt::QueuedConnection);
}

void VLCPlayer::play()
{
    libvlc_media_player_play(_vlcPlayer);
    m_bplaybackState=true;
    emit playbackStateChanged();
}

void VLCPlayer::stop()
{
    libvlc_media_player_stop(_vlcPlayer);
    m_bplaybackState=false;
    emit playbackStateChanged();
}

void VLCPlayer::pause()
{
    libvlc_media_player_pause(_vlcPlayer);
    m_bplaybackState=false;
    emit playbackStateChanged();
}

bool VLCPlayer::playbackState() const
{
    return m_bplaybackState;
}

u_int64_t VLCPlayer::position() const
{
    return libvlc_media_player_get_time(_vlcPlayer);
}

bool VLCPlayer::seekable() const {
    return libvlc_media_player_is_seekable(_vlcPlayer);
}

u_int64_t VLCPlayer::duration() const
{
    return libvlc_media_player_get_length(_vlcPlayer);
}

void VLCPlayer::setPosition(u_int64_t ms)
{
    if (_vlcPlayer)
        // Устанавливаем позицию в libvlc (значение от 0.0 до 1.0)
          libvlc_media_player_set_time(_vlcPlayer, (libvlc_time_t)ms);
}

void VLCPlayer::vlc_callback(const libvlc_event_t *event, void *data)
{
    auto self = static_cast<VLCPlayer*>(data);
    if(event->type==libvlc_MediaPlayerTimeChanged)
        // Важно: вызываем сигнал, чтобы UI обновился
        // Qt автоматически перекинет этот вызов в Main Thread через очередь событий
        emit self->positionChanged();
    else if(event->type==libvlc_MediaPlayerSeekableChanged)
        emit self->seekableChanged();
    else if (event->type == libvlc_MediaPlayerLengthChanged)
        emit self->durationChanged();
}
