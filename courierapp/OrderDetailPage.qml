import QtQuick
import QtQuick.Controls

Page {
    property int id: -1
    property string address: ""
    property string customer: ""
    property int status: -1
    property string orderdate: ""
    property int modelIndex: -1

    signal backClicked()

    header: ToolBar {
        Button { text: "Назад"; onClicked: backClicked() }
        Label { text: "Детали заказа"; anchors.centerIn: parent }
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text { text: "Заказ №" + id; font.bold: true }
        Text { text: "Дата заказа: " + orderdate }
        Text { text: "Клиент: " + customer }
        Text { text: "Адрес: " + address }
        Text { text: "Текущий статус: " + (status === 1? "Доставлен" : "Ожидание") ; color: "blue" }

        Button {
            text: "Доставлено"
            highlighted: true
            visible: status !== 1 // Показываем только если еще не доставлен
            onClicked: {
                // Вызываем метод C++, который мы создали ранее
                deliveryModel.updateStatusOnServer(modelIndex, 1)
                // Можно сразу закрыть страницу после обновления
                stack.pop()
            }
        }
    }
}
