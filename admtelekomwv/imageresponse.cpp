#include <imageresponse.h>
#include "vlcmanager.h"
#include <QTimer>

MyImageResponse::MyImageResponse(const QString &id, const QSize &requestedSize)
    : m_id(id), m_requestedSize(requestedSize) {
    setAutoDelete(false); // Управляем удалением сами
}

void MyImageResponse::run() {
    // Имитация тяжелой работы или загрузки
    QString videoPath = m_id;
    QString tempImg; // = QDir::tempPath() + "/thumb_" + QString::number(qHash(videoPath)) + ".png";

    QImage image = getFrameFromUrl(videoPath); //getVlcThumbnail(videoPath, tempImg);

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

// QImage MyImageResponse::getVlcThumbnail(QString videoPath, QString tempPath)
// {
//     const int width = 640;  // Фиксируем размер превью для скорости
//     const int height = 360;

//     libvlc_instance_t *vlc = VlcManager::instance();//libvlc_new(3, vlc_args);
//     libvlc_media_t *m = libvlc_media_new_location(vlc, videoPath.toUtf8().constData());

//     libvlc_media_add_option(m, ":no-audio");
//     libvlc_media_add_option(m, ":no-video-title-show");
//     libvlc_media_add_option(m, ":avcodec-hw=none");
//     libvlc_media_add_option(m, ":network-caching=3000"); // 3 секунды буфера для сети
//     libvlc_media_add_option(m, ":no-hw-dec");
//     libvlc_media_add_option(m, ":vout=vmem");

//     libvlc_media_player_t *mp = libvlc_media_player_new_from_media(m);

//     QImage result(width, height, QImage::Format_RGB32);
//     QEventLoop loop;
//     MyContext ctx { result.bits(), &loop, false };

//     // Настраиваем VLC на рисование в память (формат RV32 = RGBA)
//     libvlc_video_set_format(mp, "RV32", width, height, width * 4);
//     libvlc_video_set_callbacks(mp, &MyImageResponse::lock, &MyImageResponse::unlock, nullptr, &ctx);

//     ctx.canCapture = false; // Добавьте этот флаг в структуру MyContext
//     libvlc_media_player_play(mp);

//     if (videoPath.endsWith(".mp4", Qt::CaseInsensitive)) {
//         // Ждем, пока плеер реально начнет играть (до 5 секунд)
//         int waitReady = 50;
//         while (libvlc_media_player_get_state(mp) < libvlc_Playing && waitReady-- > 0) {
//             QThread::msleep(100);
//         }

//         // Теперь прыжок сработает точно
//         QThread::msleep(5000);
//         libvlc_media_player_set_time(mp, 2000); // Прыгаем на 2-ю секунду (там обычно уже нет черного кадра)
//     }
//     else
//         QThread::msleep(300);

//     // Ждем кадра 10 секунд (для сети)
//     QTimer::singleShot(10000, &loop, &QEventLoop::quit);
//     ctx.canCapture = true; // Разрешаем захват только ТЕПЕРЬ
//     loop.exec();

//     // 1. Обязательно отключаем колбэки ПЕРЕД остановкой
//     // Это гарантирует, что VLC не вызовет lock/unlock, пока mp->stop() разрушает потоки
//     libvlc_video_set_callbacks(mp, nullptr, nullptr, nullptr, nullptr);

//     libvlc_media_player_stop(mp);

//     // КРИТИЧНО: Даем VLC время реально остановить потоки, прежде чем функция выйдет
//     // и ctx на стеке уничтожится.
//     QThread::msleep(200);

//     // QImage result;
//     if (ctx.frameCaptured) {
//         return result;
//     }

//     libvlc_media_player_release(mp);
//     libvlc_media_release(m);

//     return QImage(nullptr); //result;
// }

// // Callback: VLC спрашивает, куда рисовать
// void* MyImageResponse::lock(void* data, void** planes) {
//     MyContext* ctx = static_cast<MyContext*>(data);
//     *planes = ctx->buffer; // Отдаем адрес нашего массива
//     return nullptr;
// }

// // Callback: Кадр отрисован в память
// void MyImageResponse::unlock(void* data, void*, void* const*) {
//     MyContext* ctx = static_cast<MyContext*>(data);
//     // Захватываем кадр, только если мы разрешили (после msleep в основном потоке)
//     if (ctx && ctx->canCapture && !ctx->frameCaptured) {
//         ctx->frameCaptured = true;
//         // Безопасный выход из цикла
//         ctx->loop->quit();
//         // QMetaObject::invokeMethod(ctx->loop, "quit", Qt::QueuedConnection);
//     }
// }

QImage MyImageResponse::getFrameFromUrl(QString url) {
    // libvlc_instance_t* inst = VlcManager::instance();
    const char* const vlc_args[] = {
                                    "--vout=vmem",
                                    "--avcodec-hw=none",
                                    "--no-audio",
                                    "--no-video-title-show",
                                    "--ignore-config"
    };
    libvlc_instance_t* inst =  libvlc_new(0, nullptr); //libvlc_new(5, vlc_args);

    libvlc_media_t* m = libvlc_media_new_location(inst, url.toUtf8().data());
    libvlc_media_add_option(m, ":vout=vmem");
    libvlc_media_add_option(m, ":no-audio");
    libvlc_media_add_option(m, ":no-osd");
    libvlc_media_add_option(m, ":avcodec-hw=none");
    libvlc_media_add_option(m, ":no-video-title-show");

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
            sync.targetPosition = 0.5f; // Мы хотим прыгнуть на середину

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
    libvlc_release(inst);

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
        if (std::abs(newPos - sync->targetPosition) < 0.05f) {
            sync->seekFinished = true;
        }
    }
}
