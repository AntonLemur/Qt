import QtQuick
import QtQuick.Controls

Page {
    header: ToolBar { Label { text: "Мои заказы"; anchors.centerIn: parent } }

    signal orderSelected(int id, string address, string customer, int status, string orderdate, int modelIndex)

    ListView {
        id: ordersList // Даем ID списку
        anchors.fill: parent
        model: deliveryModel

        // Локальный флаг для мгновенной блокировки
        property bool isRequestSent: false

        // Оставляем пустой отступ сверху, чтобы список не перекрывал крутилку при загрузке
        topMargin: deliveryModel.isLoading ? 60 : 0

        Behavior on topMargin {
            NumberAnimation { duration: 200 }
        }

        onContentYChanged: {
            if (dragging && contentY < -100) {
                // Проверяем и локальный флаг, и флаг бэкенда
                if (!isRequestSent && !deliveryModel.isLoading) {
                    isRequestSent = true; // Мгновенно блокируем повторный вход
                    console.log("Отправка запроса...");
                    deliveryModel.fetchOrders();
                }
            }

            // Сбрасываем локальный флаг, когда пользователь вернул список в нормальное состояние
            if (!dragging && contentY >= 0) {
                isRequestSent = false;
            }
        }

        delegate: ItemDelegate {
            // Вместо parent.width используем ID списка
            width: ordersList.width
            contentItem: Row {
                spacing: 12

                // Красивая иконка статуса
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: materialFont.name
                    font.pixelSize: 28
                    text: status === 1 ? "\ue86c" : "\ue8b5" // "check_circle" или "schedule"
                    color: status === 1 ? "#4CAF50" : "#FF9800"
                }

                Column {
                    spacing: 2

                    Row {
                        spacing: 5
                        Text {
                            text: "\ue8b0"; // иконка "label" (номер заказа)
                            font.family: materialFont.name; font.pixelSize: 14; color: "#757575"
                        }
                        Text { text: id; font.pixelSize: 12; color: "#1976D2"; font.bold: true }

                        Text { text: "  \ue192"; font.family: materialFont.name; font.pixelSize: 14; color: "#757575" }
                        Text { text: Qt.formatDateTime(orderdate, "HH:mm"); font.pixelSize: 12; color: "#757575" }
                    }

                    Text {
                        text: address
                        font.pixelSize: 16
                        font.bold: true
                        color: "#212121"
                    }

                    Row {
                        spacing: 5
                        Text {
                            text: "\ue853"; // иконка "account_circle" (клиент)
                            font.family: materialFont.name; font.pixelSize: 16; color: "#616161"
                        }
                        Text { text: customer; font.pixelSize: 14; color: "#616161" }
                    }
                }
            }
            onClicked: orderSelected(id, address, customer, status, Qt.formatDateTime(orderdate, "dd.MM.yyyy HH:mm"), index) // Генерируем сигнал
        }
    }

    // Крутилка теперь не зависит от прокрутки списка
    // BusyIndicator {
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     anchors.top: parent.top
    //     anchors.topMargin: 40 // Чуть ниже верхнего края
    //     running: deliveryModel.isLoading
    //     visible: deliveryModel.isLoading
    //     z: 10 // Гарантирует, что она будет поверх всех элементов списка
    // }

    // Размещаем крутилку фиксированно вверху страницы
    BusyIndicator {
        id: topIndicator
        anchors.horizontalCenter: parent.horizontalCenter
        y: 10 // Всегда на одном месте сверху
        running: deliveryModel.isLoading
        visible: running
        z: 2
    }

    // Временная проверка (увидим в углу статус)
    // Text {
    //     anchors.right: parent.right
    //     anchors.top: parent.top
    //     text: "Загрузка: " + deliveryModel.isLoading
    //     color: "red"
    //     z: 101
    // }

    Component.onCompleted: deliveryModel.fetchOrders()
}
