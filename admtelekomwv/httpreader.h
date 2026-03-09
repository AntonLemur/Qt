#ifndef HTTPREADER_H
#define HTTPREADER_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QVideoSink>
#include <QtQml/qqmlregistration.h> //для QML_ELEMENT

class HttpReader : public QObject
{
    Q_OBJECT
    QML_ELEMENT // This makes the class available for use/instantiation on the QML side.
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit HttpReader(QObject *parent = nullptr);
    Q_INVOKABLE void makeHttpRequest(const QString &strurl);
    void setVideoSink(QVideoSink *newVideoSink);
    QVideoSink* videoSink() const;

private slots:
    void connected();
    void readyRead();
    void disconnected();
    void errorOccurred(QTcpSocket::SocketError socketError);

signals:
    void videoSinkChanged();
private:
    QTcpSocket *socket;
    QVideoSink *m_videoSink;
    QByteArray responseBuffer;
    QString host;
    quint16 port;
    quint16     m_nNextBlockSize;
};

#endif // HTTPREADER_H
