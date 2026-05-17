#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "order.h"
#include <QTimer>

class OrderModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum OrderRoles { IdRole = Qt::UserRole + 1, AddressRole, CustomerRole, StatusRole, OrderDateRole };

    OrderModel(QObject *parent = nullptr) : QAbstractListModel(parent) {
        // Тестовые данные
        // m_orders << Order{"ул. Ленина, 10", "Иван Иванов"};
        // m_orders << Order{"пр. Мира, 54", "Анна Смирнова"};
        m_manager = new QNetworkAccessManager(this);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_orders.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_orders.count()) return QVariant();
        const Order &order = m_orders[index.row()];
        if (role == IdRole) return order.Id;
        if (role == AddressRole) return order.Address;
        if (role == CustomerRole) return order.Customer;
        if (role == StatusRole) return order.Status; // Отдаем статус заказа
        if (role == OrderDateRole) return order.OrderDate;

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[IdRole] = "id";
        roles[AddressRole] = "address";
        roles[CustomerRole] = "customer";
        roles[StatusRole] = "status";
        roles[OrderDateRole] = "orderdate";
        return roles;
    }

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

    bool isLoading() const
    {
        return m_isLoading;
    }

    Q_INVOKABLE void fetchOrders() // Метод для вызова из QML или C++
    {
        if (m_isLoading) return; // Защита от спама запросами

        m_isLoading = true;
        emit isLoadingChanged();

        QUrl url("http://xxx.xxx.xxx.xxx:xxxx/api/Data/orderscourier");
        QNetworkRequest request(url);
        // Указываем серверу, что ожидаем JSON
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        m_manager->get(request);
        connect(m_manager, &QNetworkAccessManager::finished, this, /*&OrderModel::onReplyFinished*/[this](QNetworkReply *reply){
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Ошибка сети:" << reply->errorString();
                reply->deleteLater();
                return;
            }

            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);

            if (doc.isArray()) {
                QJsonArray jsonArray = doc.array();

                // Сигнализируем о начале обновления данных
                beginResetModel();
                m_orders.clear();

                qDebug() << "Данные:";

                for (const QJsonValue &value : jsonArray) {
                    QJsonObject obj = value.toObject();
                    Order order;
                    // Предположим, у вашей структуры Order есть такие поля:
                    order.Id = obj["id"].toInt();
                    order.Address = obj["address"].toString();
                    order.Customer = obj["customer"].toString();
                    order.OrderDate = QDateTime::fromString(obj["order_Date"].toString(),Qt::ISODate);
                    order.Status = obj["status"].toInt();
                    order.Amount = obj["amount"].toDouble();

                    qDebug() << order.Id << ", " << order.Address << ", " << order.Customer << ", " << order.OrderDate << ", " << order.Status << ", " << order.Amount;

                    m_orders.append(order);
                }

                // Сигнализируем о завершении обновления
                endResetModel();
            }

            m_isLoading = false;
            emit isLoadingChanged();

            reply->deleteLater();
        });
    }

    Q_INVOKABLE void updateStatusOnServer(int row, int newStatus) {
        if (row < 0 || row >= m_orders.count()) return;

        long orderId = m_orders[row].Id;
        QUrl url(QString("http://xxx.xxx.xxx.xxx:xxxx/api/Data/UpdateStatus/%1").arg(orderId));

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        // Отправляем просто число newStatus как тело запроса (FromBody в C#)
        QByteArray body = QByteArray::number(newStatus);

        QNetworkReply *reply = m_manager->put(request, body);

        connect(reply, &QNetworkReply::finished, this, [this, reply, row, newStatus]() {
            if (reply->error() == QNetworkReply::NoError) {
                // Если сервер подтвердил, обновляем локально
                setOrderStatus(row, newStatus);
                qDebug() << "Сервер обновил статус заказа" << m_orders[row].Id;
            } else {
                qDebug() << "Ошибка обновления на сервере:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }

    void setOrderStatus(int row, int newStatus) {
        if (row < 0 || row >= m_orders.count()) return;

        m_orders[row].Status = newStatus;

        // ВАЖНО: Уведомляем QML, что данные в этой строке изменились
        QModelIndex index = createIndex(row, 0);
        emit dataChanged(index, index, {StatusRole});
    }

// private slots:
//     void onReplyFinished(QNetworkReply *reply);

signals:
   void isLoadingChanged();
private:
    QVector<Order> m_orders;
    QNetworkAccessManager *m_manager;
    bool m_isLoading = false;
};

#endif // ORDERMODEL_H
