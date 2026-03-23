#ifndef ASYNCIMAGEPROVIDER_H
#define ASYNCIMAGEPROVIDER_H
#include <QQuickImageResponse>
#include <QThreadPool>
#include "imageresponse.h"

class AsyncImageProvider : public QQuickAsyncImageProvider {
public:
    QQuickImageResponse* requestImageResponse(const QString &id, const QSize &requestedSize) override {
        MyImageResponse *response = new MyImageResponse(id, requestedSize);
        // Запускаем выполнение в глобальном пуле потоков
        QThreadPool::globalInstance()->start(response);

        qDebug() << id;

        return response;
    }
};

#endif // ASYNCIMAGEPROVIDER_H
