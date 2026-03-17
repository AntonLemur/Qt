#include "vlcplayer.h"
#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_events.h>
#include <QPainter>
#include <QVideoFrame>
#include <QDebug>

VLCPlayer::VLCPlayer(): m_format(QSize(VIDEO_WIDTH, VIDEO_HEIGHT), QVideoFrameFormat::Format_RGBA8888),
    m_streampath("rtsp://xxx.xxx.xxx.xxx:xxxx/mystream")
{
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

    if (!_vlcInstance) {
        qCritical() << "VLC: Failed to create libvlc instance! Check if plugins are in the app directory.";
        return; // Выходим, чтобы не упасть дальше
    }

    _vlcPlayer = libvlc_media_player_new(_vlcInstance);

    if (!_vlcPlayer) {
        qCritical() << "VLC: Failed to create media player!";
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
        return;
    }

    // Set the format: RGBA (4 bytes per pixel), width, height, pitch
    // libvlc_video_set_format(_vlcPlayer, "RGBA", VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);

    // Указываем функции для настройки формата (setup) и очистки (cleanup)
    libvlc_video_set_format_callbacks(_vlcPlayer, &VLCPlayer::setupFormat, &VLCPlayer::cleanupFormat);

    // Link our functions
    // В libvlc цикл работы с кадром выглядит так:
    // lock: VLC просит память для записи (вы делаете map).
    // Запись: VLC пишет пиксели в planes[0].
    // unlock: VLC закончил запись (вы делаете unmap).
    // display: VLC говорит: «Теперь этот кадр можно показать» (вы отправляете его в videoSink).
    libvlc_video_set_callbacks(_vlcPlayer, &VLCPlayer::lock, &VLCPlayer::unlock, &VLCPlayer::display, this);

    // Подписываемся на события VLC
    libvlc_event_manager_t *em = libvlc_media_player_event_manager(_vlcPlayer);
    libvlc_event_attach(em, libvlc_MediaPlayerTimeChanged, &VLCPlayer::vlc_callback, this);
    libvlc_event_attach(em, libvlc_MediaPlayerSeekableChanged, &VLCPlayer::vlc_callback, this);
    libvlc_event_attach(em, libvlc_MediaPlayerLengthChanged, &VLCPlayer::vlc_callback, this);
}

VLCPlayer::~VLCPlayer()
{
    // 1. Останавливаем воспроизведение
    if (_vlcPlayer) {
        libvlc_media_player_stop(_vlcPlayer);

        // Удаляем колбэки, чтобы VLC больше не пытался вызвать lock/display
        libvlc_video_set_callbacks(_vlcPlayer, nullptr, nullptr, nullptr, nullptr);

        // Освобождаем плеер
        libvlc_media_player_release(_vlcPlayer);
    }

    // 2. Освобождаем инстанс VLC
    if (_vlcInstance) {
        libvlc_release(_vlcInstance);
    }

    // 3. Очищаем кадр в UI
    if (m_videoSink) {
        m_videoSink->setVideoFrame(QVideoFrame());
    }

    qDebug() << "VLCPlayer destroyed and resources released";
}

void VLCPlayer::setSource(const QString &path) {
    if (!_vlcInstance) return;

    libvlc_media_t *m = libvlc_media_new_location(_vlcInstance, path.toUtf8().constData());

    // Используем асинхронный парсинг старого образца
    //libvlc_media_parse_async(m);

    // //Для rtsp
    // libvlc_media_add_option(m, ":codec=mediacodec,none"); // Использовать MediaCodec Android

    // // 1. Форсируем UDP (TCP дает задержки при потере пакетов)
    // libvlc_media_add_option(m, ":rtsp-tcp=0");

    // // 2. Минимальный сетевой кэш
    // libvlc_media_add_option(m, ":network-caching=150");

    // // 3. Отключаем джиттер-буфер (сглаживание дрожания сети)
    // libvlc_media_add_option(m, ":clock-jitter=0");

    // // 4. Опция для моментального вывода кадра
    // libvlc_media_add_option(m, ":rtsp-frame-buffer-size=0");


    // Если это сетевой файл (http/https), лучше оставить настройки по умолчанию
    // или добавить только базовый кэш.
    if (path.startsWith("http")) {
        libvlc_media_add_option(m, ":network-caching=1000"); // Увеличьте кэш для HTTP
    } else {
        // Ваши старые опции для RTSP
        libvlc_media_add_option(m, ":rtsp-tcp=0");
        libvlc_media_add_option(m, ":network-caching=150");
    }

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

    // Выбираем следующий буфер
    int nextIdx = (player->m_currentFrameIdx + 1) % 3;
    auto &frame = player->m_frames[nextIdx];

    // Проверяем валидность кадра перед использованием
    if (!frame.isValid()) {
        qWarning() << "VLC: Frame is not valid!";
        planes[0] = nullptr;
        return nullptr;
    }

    // Пробуем получить доступ к памяти
    if (frame.map(QVideoFrame::WriteOnly)) {
        planes[0] = frame.bits(0);
        player->m_currentFrameIdx = nextIdx; // Фиксируем индекс только при успехе
    } else {
        qWarning() << "VLC: Failed to map QVideoFrame!";
        planes[0] = nullptr;
    }

    return nullptr;
}

void VLCPlayer::unlock(void *opaque, void *picture, void *const *planes) {
    VLCPlayer *player = static_cast<VLCPlayer*>(opaque);

    // Завершаем запись здесь!
    auto &frame = player->m_frames[player->m_currentFrameIdx];

    if (frame.isMapped()) {
        frame.unmap();
    }
}

void VLCPlayer::display(void *opaque, void *picture) {
    VLCPlayer *player = static_cast<VLCPlayer*>(opaque);

    // Делаем поверхностную копию (implicit sharing)
    QVideoFrame frameToDisplay = player->m_frames[player->m_currentFrameIdx];

    if (!frameToDisplay.isValid()) return;

    QMetaObject::invokeMethod(player, [player, frameToDisplay]() {
        // Проверка m_videoSink внутри лямбды (выполняется в Main Thread)
        if (player->m_videoSink) {
            player->m_videoSink->setVideoFrame(frameToDisplay);
        }
    }, Qt::QueuedConnection);
}

void VLCPlayer::play()
{
    if (!_vlcPlayer) return;

    libvlc_media_player_play(_vlcPlayer);
    m_bplaybackState=true;
    emit playbackStateChanged();
}

void VLCPlayer::stop()
{
    if (!_vlcPlayer) return;

    libvlc_media_player_stop(_vlcPlayer);
    m_bplaybackState=false;
    emit playbackStateChanged();
}

void VLCPlayer::pause()
{
    if (!_vlcPlayer) return;

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
    if (!_vlcPlayer) return 0;
    return libvlc_media_player_get_time(_vlcPlayer);
}

bool VLCPlayer::seekable() const {
    if (!_vlcPlayer) return false;
    return libvlc_media_player_is_seekable(_vlcPlayer);
}

u_int64_t VLCPlayer::duration() const
{
    if (!_vlcPlayer) return 0;
    return libvlc_media_player_get_length(_vlcPlayer);
}

void VLCPlayer::setPosition(u_int64_t ms)
{
    if (!_vlcPlayer) return;
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

unsigned VLCPlayer::setupFormat(void **opaque, char *chroma,
                            unsigned *width, unsigned *height,
                            unsigned *pitches, unsigned *lines)
{
    auto *self = static_cast<VLCPlayer*>(*opaque);

    // Используем "RV32" (32-битный RGBA/BGRA). VLC лучше всего конвертирует в него.
    memcpy(chroma, "RV32", 4);

    // Сохраняем размеры и создаем формат для Qt VideoSink
    QSize videoSize(*width, *height);

    // RV32 выдаёт данные в формате BGRA (Blue-Green-Red-Alpha), задаём здесь такой же, чтобы каналы не путались,
    // иначе красный превращается в синий, а зеленый смешивается с ними, создавая инвертированные или смещенные цвета
    self->m_format = QVideoFrameFormat(videoSize, QVideoFrameFormat::Format_BGRA8888); //QVideoFrameFormat::Format_RGBA8888

    // Указываем VLC шаг (pitch) — сколько байт в одной строке
    *pitches = (*width) * 4;
    // Указываем количество строк
    *lines = *height;

    // Предварительно создаем 3 кадра, чтобы не выделять память в lock
    for(int i = 0; i < 3; ++i) {
        self->m_frames[i] = QVideoFrame(self->m_format);
    }

    qDebug() << "VLC Setup Format:" << videoSize << "Chroma:" << chroma;

    return 1; // Возвращаем количество плоскостей (для RGBA это всегда 1)
}

void VLCPlayer::cleanupFormat(void *opaque)
{
    // Здесь можно освободить ресурсы, если вы выделяли их специально под формат
    qDebug() << "VLC Cleanup Format";
}

QString VLCPlayer::streampath() const
{
    return m_streampath;
}
