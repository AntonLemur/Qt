#ifndef ORDERHANDLER_H
#define ORDERHANDLER_H

#include <QObject>
#include <QString>

class OrderHandler : public QObject {
    Q_OBJECT
    // Делаем свойство доступным для чтения и уведомления в QML
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit OrderHandler(QObject *parent = nullptr) : QObject(parent), m_status("Ожидание") {}

    QString status() const { return m_status; }

    // Метод, который мы вызовем из QML
    Q_INVOKABLE void completeOrder() {
        m_status = "Доставлено!";
        emit statusChanged(); // Уведомляем интерфейс об изменении
    }

signals:
    void statusChanged();

private:
    QString m_status;
};

#endif // ORDERHANDLER_H
