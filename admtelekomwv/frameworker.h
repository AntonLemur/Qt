#ifndef FRAMEWORKER_H
#define FRAMEWORKER_H
#include <QObject>
#include <QVideoFrameFormat>
#include <QVideoFrame>

// FrameWorker.h - Вспомогательный объект
class FrameWorker : public QObject {
    Q_OBJECT
public slots:
    void processFrame(const QByteArray &data, int width, int height) {
        QVideoFrameFormat format(QSize(width, height), QVideoFrameFormat::Format_RGBA8888);
        QVideoFrame frame(format);

        if (frame.map(QVideoFrame::WriteOnly)) {
            // Копируем данные из байтового массива в видеокадр
            memcpy(frame.bits(0), data.constData(), data.size());
            frame.unmap();
            emit frameReadyForSink(frame); // Отправляем готовый кадр в Sink
        }
    }
signals:
    void frameReadyForSink(QVideoFrame frame);
};

#endif // FRAMEWORKER_H
