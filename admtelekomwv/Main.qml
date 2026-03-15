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
                anchors.fill: parent
                spacing: 0

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

                    // Предполагается, что FrameProvider зарегистрирован как "com.myapp 1.0 FrameProvider"
                    // HttpReader {
                    //     id: provider
                    //     videoSink: videoOutput.videoSink
                    // }

                    // MediaPlayer {
                    //     id: mediaPlayer

                    //     // source:
                    //         // "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
                    //         // "rtsp://xxx.xxx.xxx.xxx:xxxx/mystream" //"http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
                    //         // "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                    //     // videoOutput: videoOutput
                    //     audioOutput: AudioOutput {} // Необходимо для звука

                    //     onErrorOccurred: {
                    //         console.error("source: ", mediaPlayer.source);
                    //         console.error("MediaPlayer error:", mediaPlayer.error, mediaPlayer.errorString);
                    //     }
                    // }

                    VLCPlayer {
                        id: vlcplayer
                        // anchors.fill: parent
                        // source: "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                        videoSink: videoOutput.videoSink
                    }

                    VideoOutput {
                        id: videoOutput
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectFit // Сохраняет пропорции

                        MouseArea {
                            anchors.fill: parent
                            //для MediaPlayer
                            // onClicked: {
                            //     if (mediaPlayer.playbackState === MediaPlayer.PlayingState)
                            //         mediaPlayer.pause();
                            //     else
                            //         mediaPlayer.play();
                            // }

                            onClicked: {
                                if (vlcplayer.playbackState)
                                    vlcplayer.pause();
                                else
                                    vlcplayer.play();
                            }
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
                        mediaPlayer: vlcplayer //mediaPlayer

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
                        mediaPlayer: vlcplayer //mediaPlayer

                        // isPlaylistVisible: playlistInfo.visible

                        // onPlayNextFile: {
                        //     if (playlistInfo.mediaCount) {
                        //         if (!playlistInfo.isShuffled){
                        //             ++root.currentFile
                        //             if (root.currentFile > playlistInfo.mediaCount - 1 && root.playlistLooped) {
                        //                 root.currentFile = 0
                        //             } else if (root.currentFile > playlistInfo.mediaCount - 1 && !root.playlistLooped) {
                        //                 --root.currentFile
                        //                 return
                        //             }
                        //         }
                        //         root.playMedia()
                        //     }
                        // }

                        // onPlayPreviousFile: {
                        //     if (playlistInfo.mediaCount) {
                        //         if (!playlistInfo.isShuffled){
                        //             --root.currentFile
                        //             if (root.currentFile < 0 && isPlaylistLooped) {
                        //                 root.currentFile = playlistInfo.mediaCount - 1
                        //             } else if (root.currentFile < 0 && !root.playlistLooped) {
                        //                 ++root.currentFile
                        //                 return
                        //             }
                        //         }
                        //         root.playMedia()
                        //     }
                        // }

                        // playlistButton.onClicked: !playlistInfo.visible ? root.showOverlay(playlistInfo) : root.closeOverlays()
                        // menuButton.onClicked: menuPopup.open()
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

                // Component.onCompleted: mediaPlayer.play()
                // Component.onCompleted: provider.makeHttpRequest(
                //     "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
                Component.onCompleted: vlcplayer.setSource(
                    // "rtsp://xxx.xxx.xxx.xxx:xxxx/mystream"
                    // "https://stream.mux.com/v69RSHhFelSm4701snP22dYz2jICy4E4FUyk02rW4gxRM.m3u8"
                    // "https://xxx.xxx.xxx.xx/hls/xxx/xxx.m3u8?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx&token=xxxxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                    "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
                    )
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

                                        // Формируем путь: префикс + путь к файлу
                                        source: "image://thumb/" + filePath

                                        // Опционально: пока грузится превью, покажем серый фон
                                        Rectangle {
                                            anchors.fill: parent
                                            color: "#333"
                                            visible: parent.status != Image.Ready
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
