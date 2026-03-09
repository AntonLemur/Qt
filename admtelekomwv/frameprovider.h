#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H
#include <QObject>
#include <QWebSocket>
#include <QVideoSink>
#include <QVideoFrame>
#include <QtQml/qqmlregistration.h> //для QML_ELEMENT

class FrameProvider : public QObject {
    Q_OBJECT
    QML_ELEMENT // This makes the class available for use/instantiation on the QML side.
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit FrameProvider(QObject *parent = nullptr) : QObject(parent) {
        m_videoSink = new QVideoSink(this);
        connect(&m_webSocket, &QWebSocket::connected, this, &FrameProvider::onConnected);
        connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &FrameProvider::onFrameReceived);
        m_webSocket.open(QUrl("ws://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"));//("https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    }

    void setVideoSink(QVideoSink *newVideoSink)
    {
        if (m_videoSink == newVideoSink)
            return;
        m_videoSink = newVideoSink;
        emit videoSinkChanged();
    }
    QVideoSink* videoSink() const { return m_videoSink; }

private slots:
    void onConnected() {
        qDebug() << "Connected to streaming server";
    }

    void onFrameReceived(const QByteArray &data) {
        // 1. Декодировать данные (data -> QImage -> QVideoFrame)
        qDebug() << "onFrameReceived...";

        QImage img = QImage::fromData(data);
        QVideoFrame frame(img);

        // 2. Отправить кадр в sink
        m_videoSink->setVideoFrame(frame);
    }

signals:
    void videoSinkChanged();

private:
    QWebSocket m_webSocket;
    QVideoSink *m_videoSink;
};

#endif // FRAMEPROVIDER_H
