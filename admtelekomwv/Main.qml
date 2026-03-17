import QtQuick
import QtWebView
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtMultimedia
import MediaControls
import Config

ApplicationWindow  {
    id: mainwindow
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    function getTime(time : int) : string {
        const h = Math.floor(time / 3600000).toString()
        const m = Math.floor(time / 60000).toString()
        const s = Math.floor(time / 1000 - m * 60).toString()
        return `${h.padStart(2,'0')}:${m.padStart(2,'0')}:${s.padStart(2, '0')}`
    }

    // The StackView manages the navigation
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainPage // Set the initial page

        // Optional: Add an attached property to provide access to the stackView from within the pushed items
        // You might use a signal or a more structured approach for larger apps,
        // but this works for a simple example.
    }

    // --- Page Definitions ---

    // Page 1: Main Page
    Component {
        id: mainPage
        Page {
            title: "Мой двор"
            ColumnLayout {
                id: rootLayout
                anchors.fill: parent
                spacing: 0

                property string thumbSource: "" // Изначально пусто
                property bool checkingUrl: true // Флаг для начальной проверки
                // 1. Определение собственного сигнала
                signal setsource(string url)
                // 3. Обработка сигнала
                onSetsource: (url) => {
                    vlcplayer.setSource(url)
                }


                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    Text { anchors.centerIn: parent; text: "Мой двор"; font.pointSize: 24 }
                }

                RowLayout {
                     id: topToolbar
                     Layout.fillWidth: true
                     Layout.preferredHeight: 20
                     spacing: 10
                     Button {
                         id: archiveButton
                         Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                         text: "Архив"
                         onClicked: {
                             stackView.push(archivePage)
                         }
                     }
                     Item {
                         id: archivename
                         Layout.fillWidth: true
                         Layout.fillHeight: true
                     }
                }

                Rectangle
                {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200

                    // 1. Компонент видео
                    VLCPlayer {
                        id: vlcplayer // для использования внутри этой ветки
                        objectName: "VLCPlayer" // для поиска из C++, а потом использования в других ветках
                        videoSink: videoOutput.videoSink
                    }

                    VideoOutput {
                        id: videoOutput
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectFit // Сохраняет пропорции

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                if (vlcplayer.playbackState)
                                    vlcplayer.pause();
                                else
                                    vlcplayer.play();
                            }
                        }
                    }

                    // 2. Элемент миниатюры
                    Image {
                        id: thumbnail
                        anchors.fill: parent
                        source: { //"image://thumb/" + vlcplayer.streampath // Путь к вашей картинке
                            if(!vlcplayer.playbackState)
                                return rootLayout.thumbSource;
                            else {
                                rootLayout.thumbSource=""
                                return rootLayout.thumbSource
                            }
                        }
                        fillMode: Image.PreserveAspectFit
                        visible: !vlcplayer.playbackState // По умолчанию показываем
                        asynchronous: true

                        // Скрывать при клике
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                parent.visible = false;
                                vlcplayer.play();
                            }
                        }

                        // Индикатор загрузки, пока идет проверка или генерация
                        BusyIndicator {
                            anchors.centerIn: parent
                            running: (thumbnail.status === Image.Loading) || (rootLayout.checkingUrl === true)
                        }
                    }
               }

               Rectangle
               {
                   id: rectSeek
                   Layout.fillWidth: true
                   Layout.preferredHeight: 30

                   PlaybackSeekControl {
                        id: seeker
                        mediaPlayer: vlcplayer

                        fullScreenButton.onClicked: {
                            if (mediaPlayer.hasVideo) {
                                videoOutput.fullScreen ?  root.showNormal() : root.showFullScreen()
                                videoOutput.fullScreen = !videoOutput.fullScreen
                            }
                        }

                        settingsButton.onClicked: !settingsInfo.visible ? root.showOverlay(settingsInfo) : root.closeOverlays()
                    }
               }

               Rectangle
               {
                   Layout.fillWidth: true
                   Layout.preferredHeight: 30

                   PlaybackControl {
                        id: playbackControl
                        mediaPlayer: vlcplayer
                    }
               }

               RowLayout {
                    id: mainToolbar
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    spacing: 10
                    Button {
                        id: customButton
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        text: "Перейти в кабинет" //"Go to your account"
                        contentItem: ColumnLayout {
                                spacing: 10 // Space between the icon and the text

                                Image {
                                    fillMode: Image.PreserveAspectFit // Картинка впишется, без обрезки
                                    source: "qrc:/images/profile"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Label {
                                    id: textLabel
                                    text: customButton.text // Bind the Label's text to the Button's text property
                                    Layout.alignment: Qt.AlignHCenter
                                    font: customButton.font
                                }
                        }
                        onClicked: {
                            // Push the detailPage component onto the stack
                            stackView.push(detailPage)
                        }
                    }
                    Item {
                        id: name
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
               }

                Component.onCompleted: {
                    checkUrl(vlcplayer.streampath)
                    // vlcplayer.setSource(
                    //     // "rtsp://xxx.xxx.xxx.xxx:xxxx/mystream"
                    //     // "https://stream.mux.com/v69RSHhFelSm4701snP22dYz2jICy4E4FUyk02rW4gxRM.m3u8"
                    //     // "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                    //     // "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
                    // )
                }

                function checkUrl(url) {
                    rootLayout.checkingUrl = true;
                    var xhr = new XMLHttpRequest();
                    xhr.open("GET", url);
                    xhr.setRequestHeader("Range", "bytes=0-10"); // Минимум данных

                    xhr.onreadystatechange = function() {
                        if (xhr.readyState === XMLHttpRequest.DONE) {
                            rootLayout.checkingUrl = false; // ОСТАНАВЛИВАЕМ ИНДИКАТОР
                            if (xhr.status === 200 || xhr.status === 206) {
                                rootLayout.thumbSource = "image://thumb/" + url;
                                rootLayout.setsource(vlcplayer.streampath);
                            } else {
                                console.log("Ошибка! Статус 0 означает блок сети или CORS");
                                rootLayout.thumbSource = "qrc:/images/error-icon.png"; // Заглушка
                            }
                        }
                    };
                    xhr.send();
                }
            }
        }
    }

    // Page 2: Detail Page
    Component {
        id: detailPage
        Page {
            title: "Кабинет"

            ColumnLayout {
                anchors.fill: parent

                WebView {
                    id: webview
                    // anchors.fill: parent
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    url: "https://adm174.ru" // Адрес вашего сайта
                    focus: true
                    settings.javaScriptEnabled: true
                    settings.localStorageEnabled: true // Важно для многих сайтов
                    settings.allowFileAccess: true
                    settings.localContentCanAccessFileUrls: true

                    Component.onCompleted: forceActiveFocus()

                    onLoadingChanged: {
                        console.log("Can go back:", webview.canGoBack, focus);
                    }

                    Shortcut {
                        sequences: [StandardKey.Back]
                        onActivated: if (webview.canGoBack)
                                        webview.goBack();
                    }
                }

                Button {
                    id: customButton
                    text: "Перейти в Мой двор"
                    contentItem:  ColumnLayout {
                        spacing: 10 // Space between the icon and the text

                        Image {
                            source: "qrc:/images/cam.png"
                            fillMode: Image.PreserveAspectFit // Картинка впишется, без обрезки
                            anchors.centerIn: parent
                        }
                        Label {
                            id: textLabel
                            text: customButton.text // Bind the Label's text to the Button's text property
                            Layout.alignment: Qt.AlignVCenter
                            font: customButton.font
                            color: customButton.color // Adjust color based on button state (e.g. hovered, pressed)
                        }
                    }
                    onClicked: {
                        // Pop the current page off the stack
                        stackView.pop()
                    }
                }
            }
        }
    }

    // Page 3: Archive Page
    Component {
        id: archivePage
        Page {
            title: "Движения"

            ColumnLayout {
                anchors.fill: parent
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 5 // Add spacing between items
                    model: ListModel {
                        id: pathModel

                        ListElement {
                            filePath: "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
                        }
                        ListElement {
                            filePath: "https://stream.mux.com/v69RSHhFelSm4701snP22dYz2jICy4E4FUyk02rW4gxRM.m3u8"
                        }
                        ListElement {
                            filePath: "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                        }
                    }// Источник путей к файлам

                    delegate: Row {
                                    spacing: 10

                                    Image {
                                        width: 160; height: 90
                                        fillMode: Image.PreserveAspectCrop
                                        asynchronous: true
                                        cache: false

                                        // Формируем путь: префикс + путь к файлу
                                        source: "image://thumb/" + filePath

                                        // Опционально: пока грузится превью, покажем серый фон
                                        Rectangle {
                                            anchors.fill: parent
                                            color: "#333"
                                            visible: parent.status != Image.Ready
                                        }

                                        // запускать воспроизведение
                                        MouseArea {
                                            anchors.fill: parent
                                            Timer {
                                                id: myTimer
                                                interval: 50
                                                onTriggered: {
                                                    globalVLCPlayer.play();
                                                    stackView.pop();
                                                }
                                            }

                                            onClicked: {
                                                // 1. Сначала меняем данные
                                                globalVLCPlayer.stop()
                                                globalVLCPlayer.setSource(filePath);

                                                // 2. Делаем небольшую задержку перед переходом,
                                                // чтобы ImageProvider-ы успели отработать или закрыться
                                                myTimer.start()
                                            }
                                        }
                                    }

                                    // Text element bound to the 'filePath' role
                                    // Text {
                                    //     text: filePath
                                    //     font.pixelSize: 18
                                    //     anchors.verticalCenter: parent.verticalCenter
                                    // }
                                }
                }

                Button {
                    id: archivecustomButton
                    text: "Перейти в Мой двор"
                    contentItem:  ColumnLayout {
                        spacing: 10 // Space between the icon and the text

                        Image {
                            source: "qrc:/images/cam.png"
                            fillMode: Image.PreserveAspectFit // Картинка впишется, без обрезки
                            anchors.centerIn: parent
                        }
                        Label {
                            id: archivetextLabel
                            text: archivecustomButton.text // Bind the Label's text to the Button's text property
                            Layout.alignment: Qt.AlignVCenter
                            font: archivecustomButton.font
                            color: archivecustomButton.color // Adjust color based on button state (e.g. hovered, pressed)
                        }
                    }
                    onClicked: {
                        // Pop the current page off the stack
                        stackView.pop()
                    }
                }
            }
        }
    }
}
