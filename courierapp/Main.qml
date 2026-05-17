import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Window {
    id: window
    visible: true
    width: 360
    height: 640

    //закгрузка иконок
    FontLoader {
        id: materialFont
        source: "qrc:/fonts/MaterialIcons-Regular.ttf"
        onStatusChanged: {
            if (status == FontLoader.Ready) {
                console.log("Шрифт загружен! Имя для использования:", name)
            } else if (status == FontLoader.Error) {
                console.log("Ошибка загрузки! Проверьте путь.")
            }
        }
    }

    StackView {
        id: stack
        anchors.fill: parent
        // Первой загружаем страницу со списком
        initialItem: orderListScene
    }

    // Компонент списка заказов
    Component {
        id: orderListScene
        OrderListPage {
            onOrderSelected: (id, address, customer, status, orderdate, modelIndex) => {
                stack.push(orderDetailScene, {
                    "id": id,  // Передаем индекс строки
                    "address": address,
                    "customer": customer,
                    "status": status,    // И текущий статус
                    "orderdate": orderdate,
                    "modelIndex": modelIndex
                })
            }
        }
    }

    // Компонент деталей заказа
    Component {
        id: orderDetailScene
        OrderDetailPage {
            // Кнопка "Назад" вернет нас к списку
            onBackClicked: stack.pop()
        }
    }
}
