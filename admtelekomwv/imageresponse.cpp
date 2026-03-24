#include <imageresponse.h>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include "vlcmanager.h"

MyImageResponse::MyImageResponse(const QString &id, const QSize &requestedSize)
    : m_id(id), m_requestedSize(requestedSize) {
    setAutoDelete(false); // Управляем удалением сами
}

void MyImageResponse::run() {
    // Имитация тяжелой работы или загрузки
    QString videoPath = m_id;

    bool isStream = videoPath.endsWith(".m3u8") || videoPath.startsWith("rtsp://");
    QString cachePath;

    if(!isStream)
    {
        // 1. Создаем уникальное имя файла на основе хэша URL
        QString hash = QString(QCryptographicHash::hash(videoPath.toUtf8(), QCryptographicHash::Md5).toHex());
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbs/";
        QDir().mkpath(cacheDir);
        cachePath = cacheDir + hash + ".png";

        // 2. Проверяем, есть ли уже такая картинка на диске
        if (QFile::exists(cachePath)) {
            QImage cachedImg(cachePath);
            if (!cachedImg.isNull()) {
                m_requestedSize = cachedImg.size();
                // Обязательно масштабируйте до requestedSize, если он задан!
                if (m_requestedSize.isValid()) {
                    cachedImg = cachedImg.scaled(m_requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                m_image = cachedImg;
                emit finished(); // Сигнализируем QML, что картинка готова
            }
        }
    }

    // 3. Если кэша нет, запускаем LibVLC
    QImage image = getFrameFromUrl(videoPath);

    // 4. Сохраняем результат в кэш для следующего раза
    if (!image.isNull() && isStream) {
        image.save(cachePath, "PNG");
    }

    // Обязательно масштабируйте до requestedSize, если он задан!
    if (m_requestedSize.isValid()) {
        image = image.scaled(m_requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_image = image;
    emit finished(); // Сигнализируем QML, что картинка готова
}

QQuickTextureFactory* MyImageResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QImage MyImageResponse::getFrameFromUrl(QString url) {
    libvlc_instance_t* inst = VlcManager::instance();

    libvlc_media_t* m = libvlc_media_new_location(inst, url.toUtf8().data());
    libvlc_media_add_option(m, ":vout=vmem");
    libvlc_media_add_option(m, ":no-audio");
    libvlc_media_add_option(m, ":no-osd");
    libvlc_media_add_option(m, ":avcodec-hw=none");
    libvlc_media_add_option(m, ":no-video-title-show");
    libvlc_media_add_option(m, ":ignore-config");

    // libvlc_media_add_option(m, ":network-caching=3000"); // Даем 1.5 сек на буфер
    // libvlc_media_add_option(m, ":no-hwdec");            // Еще раз запрещаем железо
    // libvlc_media_add_option(m, ":codec=avcodec");       // Только программный кодек
    // libvlc_media_add_option(m, ":network-caching=0"); // Отключаем кэш для миниатюр
    // libvlc_media_add_option(m, ":no-input-fast-seek"); // Более точная (хоть и медленная) перемотка


    libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);

    MyContext ctx;
    int width = 640;  // Желаемый размер миниатюры
    int height = 360;
    ctx.buffer.resize(width * height * 4); // RGBA

    // Устанавливаем формат вывода "в память" (RV32 = RGBA)
    libvlc_video_set_format(mp, "RV32", width, height, width * 4);
    libvlc_video_set_callbacks(mp, &MyImageResponse::lock, nullptr, &MyImageResponse::display, &ctx);

    // 1. Начинаем играть
    libvlc_media_player_play(mp);

    // 2. Ждем реального начала воспроизведения (Playing)
    // Без этого set_position не сработает!
    for(int i = 0; i < 100; ++i) {
        libvlc_state_t state = libvlc_media_player_get_state(mp);
        if (state == libvlc_Playing) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // // Ждем, пока VLC узнает длину видео (это признак готовности)
    // bool ready = false;
    // for(int i = 0; i < 200; ++i) {
    //     if (libvlc_media_player_get_length(mp) > 0) {
    //         ready = true;
    //         break;
    //     }
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // }

    // if (ready)
    // {
        if (url.endsWith(".mp4", Qt::CaseInsensitive)) {

            VlcSyncContext sync;
            sync.targetPosition = 0.1f; // Мы хотим прыгнуть на середину

            // Получаем менеджер событий плеера
            libvlc_event_manager_t* em = libvlc_media_player_event_manager(mp);

            // Подписываемся на событие изменения позиции
            libvlc_event_attach(em, libvlc_MediaPlayerPositionChanged, &MyImageResponse::vlc_callback, &sync);

            // Сбрасываем флаг перед перемоткой
            // ctx.frameCaptured = false;

            // Перематываем
            libvlc_media_player_set_position(mp, 0.1f);

            // Ждем подтверждения от события (с тайм-аутом)
            for (int i = 0; i < 100; ++i) {
                if (sync.seekFinished.load()) {
                    qDebug() << "Seek confirmed by event at step" << i;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            ctx.frameCaptured = false;

            // 3. Ждем именно НОВОГО кадра после перемотки
            // Важно: даем чуть больше времени, так как декодирование после прыжка тяжелее
            for(int i = 0; i < 100; ++i) {
                if (ctx.frameCaptured.load()) {
                    qDebug() << "SUCCESS: Captured at step" << i;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if (i % 10 == 0) qDebug() << "Still waiting... step" << i;
            }

            // Не забываем отписаться в конце
            libvlc_event_detach(em, libvlc_MediaPlayerPositionChanged, &MyImageResponse::vlc_callback, &sync);
        } else {
            // Сбрасываем флаг перед перемоткой
            ctx.frameCaptured = false;
            // Для live-потоков просто ждем первый стабильный кадр
            for(int i = 0; i < 100 && !ctx.frameCaptured; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    // }

    libvlc_media_player_stop(mp);

    QImage finalImg;
    if (ctx.frameCaptured.load()) {
        qDebug() << url.right(4) << " - First byte of buffer:" << (int)ctx.buffer[0]
                 << (int)ctx.buffer[1] << (int)ctx.buffer[2];

        QImage temp(ctx.buffer.data(), width, height, QImage::Format_RGBA8888);

        if (temp.isNull()) {
            qWarning() << url.right(4) << " - QImage failed to construct from buffer!";
        } else {
            finalImg =  temp.rgbSwapped().copy(); // Это превратит BGR в RGB
            qDebug() << url.right(4) << " - QImage size:" << finalImg.size() << "Bytes:" << finalImg.sizeInBytes();
            // Сохраните для теста во внутреннюю память телефона
            // finalImg.save("/sdcard/Download/test.png");
        }
    }

    libvlc_media_player_release(mp);

    return finalImg;
}

// 1. Колбэк: подготовка буфера для кадра
void* MyImageResponse::lock(void* data, void** p_pixels) {
    MyContext* ctx = static_cast<MyContext*>(data);
    *p_pixels = ctx->buffer.data();
    return nullptr;
}

// 2. Колбэк: когда кадр готов (здесь мы понимаем, что картинка есть)
void MyImageResponse::display(void* data, void* id) {
    MyContext* ctx = static_cast<MyContext*>(data);
    ctx->frameCaptured = true;
}

void MyImageResponse::vlc_callback(const libvlc_event_t* event, void* data) {
    auto* sync = static_cast<VlcSyncContext*>(data);

    if (event->type == libvlc_MediaPlayerPositionChanged) {
        float newPos = event->u.media_player_position_changed.new_position;
        // Если новая позиция близка к целевой (с учетом погрешности)
        if (std::abs(newPos - sync->targetPosition) < 0.005f) {
            sync->seekFinished = true;
        }
    }
}
