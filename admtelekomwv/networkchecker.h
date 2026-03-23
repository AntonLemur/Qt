#ifndef NETWORKCHECKER_H
#define NETWORKCHECKER_H

#include <QObject>
#include <QTcpSocket>
#include <QtQml/qqmlregistration.h> //для QML_ELEMENT

class NetworkChecker : public QObject
{
    Q_OBJECT
    QML_ELEMENT // This makes the class available for use/instantiation on the QML side.
public:
    explicit NetworkChecker(QObject *parent = nullptr) : QObject(parent) {}

    // Метод для вызова из QML
    Q_INVOKABLE void checkPort(const QString &host, int port) {
        QTcpSocket *socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::connected, this, [this, socket, host]() {
            emit portStatus(host, true);
            socket->deleteLater();
        });

        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
                this, [this, socket, host](QAbstractSocket::SocketError) {
                    emit portStatus(host, false);
                    socket->deleteLater();
                });

        socket->connectToHost(host, port);
        // Таймаут 2 секунды, чтобы не ждать вечно
        if (!socket->waitForConnected(2000)) {
            emit portStatus(host, false);
            socket->abort();
        }
    }

signals:
    void portStatus(QString host, bool isAvailable);
};

#endif // NETWORKCHECKER_H
