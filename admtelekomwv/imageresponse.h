#ifndef IMAGERESPONSE_H
#define IMAGERESPONSE_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>
#include <QRunnable>
#include <vlc/vlc.h>
#include <QDir>
#include <QEventLoop>
#include <atomic>

// struct MyContext {
//     uchar* buffer; // Массив для пикселей
//     QEventLoop* loop;
//     bool frameCaptured = false;
//     bool canCapture = false;
//     // int skipFrames; // Счётчик кадров для пропуска
// };

struct MyContext {
    std::vector<uchar> buffer;
    std::atomic<bool> frameCaptured{false};
};

// Структура для передачи данных в колбэк
struct VlcSyncContext {
    std::atomic<bool> seekFinished{false};
    float targetPosition = 0.0f;
};

class MyImageResponse : public QQuickImageResponse, public QRunnable {
public:
    MyImageResponse(const QString &id, const QSize &requestedSize);

    void run() override;

    QQuickTextureFactory* textureFactory() const override;

    QString m_id;
    QSize m_requestedSize;
    QImage m_image;

private:
    // QImage getVlcThumbnail(QString videoPath, QString tempPath);
    QImage getFrameFromUrl(QString url);
    static void* lock(void* data, void** planes);
    // static void unlock(void* data, void*, void* const*);
    static void display(void* data, void* id);
    static void vlc_callback(const libvlc_event_t* event, void* data);
};

#endif // IMAGERESPONSE_H
