#include "httpreader.h"
#include <QDebug>
#include <QStringBuilder>
#include <QUrl>

HttpReader::HttpReader(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);
    m_videoSink = new QVideoSink(this);
    connect(socket, &QTcpSocket::connected, this, &HttpReader::connected);
    connect(socket, &QTcpSocket::readyRead, this, &HttpReader::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &HttpReader::disconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &HttpReader::errorOccurred);
}

void HttpReader::makeHttpRequest(const QString &strurl)
{
    host = QUrl(strurl).host();
    port = QUrl(strurl).port() == -1 ? 80 : QUrl(strurl).port(); // Default to port 80 for HTTP
    responseBuffer.clear();
    socket->connectToHost(host, port);
    qDebug() << "Connecting to host:" << host << "port:" << port;}

void HttpReader::connected()
{
    qDebug() << "Connected!";
    // Send an HTTP GET request
    QByteArray request;
    request.append(QString("GET /hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx HTTP/1.1\r\n")
                       .arg(host)
                       .toUtf8());
    request.append("Host: " + host.toUtf8() + "\r\n");
    // request.append("Connection: close\r\n"); // Request the server to close the connection after the response
    request.append("\r\n"); // End of request header

    socket->write(request);
    socket->flush(); // Ensure data is written
}

void HttpReader::readyRead()
{
    responseBuffer.clear();
    // Append new data to the buffer
    qDebug() << "Data available (bytes):" << socket->bytesAvailable();
    responseBuffer.append(socket->readAll());

    // Simple parsing: check for the end of the headers (\r\n\r\n)
    if (responseBuffer.contains("\r\n\r\n")) {
        qDebug() << "Full headers received. Data size:" << responseBuffer.size();

        // In a real application, you would parse headers to get Content-Length
        // and only process the body after all bytes are received.
        // For this example with "Connection: close", we can wait for disconnection.

        // If you want to process immediately, split headers and body:
        int headerEndIndex = responseBuffer.indexOf("\r\n\r\n") + 4;
        QByteArray headers = responseBuffer.left(headerEndIndex);
        QByteArray body = responseBuffer.mid(headerEndIndex);

        qDebug() << "Headers:\n" << headers;
        qDebug() << "Body:\n" << body;
    }
    else
    {
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_2);
        for (;;) {
            if (!m_nNextBlockSize) {
                if (socket->bytesAvailable() < sizeof(quint16)) {
                    break;
                }
                in >> m_nNextBlockSize;
            }

            if (socket->bytesAvailable() < m_nNextBlockSize) {
                break;
            }
            QString str;
            in >> str;

            qDebug() << str;
            m_nNextBlockSize = 0;
        }
    }
}

void HttpReader::disconnected()
{
    qDebug() << "Disconnected by server. Total bytes received:" << responseBuffer.size();
    // qDebug() << "Full Response:\n" << responseBuffer;
    // Process the full buffer here if you relied on the server closing the connection
    responseBuffer.clear();
}

void HttpReader::errorOccurred(QTcpSocket::SocketError socketError)
{
    qDebug() << "Error:" << socket->errorString();
}

void HttpReader::setVideoSink(QVideoSink *newVideoSink)
{
    if (m_videoSink == newVideoSink)
        return;
    m_videoSink = newVideoSink;
    emit videoSinkChanged();
}

QVideoSink* HttpReader::videoSink() const
{
    return m_videoSink;
}
