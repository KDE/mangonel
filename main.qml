import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import org.kde 1.0
import QtGraphicalEffects 1.0

Window {
    id: window
    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    color: "transparent"

    width: 800
    height: 400
    visible: true

    onVisibleChanged:{
        inputText.text = ""
        if (visible) {
            requestActivate()
        }

        inputText.preHistoryText = ""
        inputText.historyIndex = -1
    }
    onActiveChanged: if (!active) visible = false



    Connections {
        target: Mangonel
        onTriggered: window.visible = true
    }

    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: shadow.radius
        }

        color: Qt.rgba(0, 0, 0, 0.5)
        radius: 10
        visible: false

        Rectangle {
            id: bottomBackground

            anchors {
                bottom: background.bottom
                left: background.left
                right: background.right
            }
            color: "black"
            height: inputText.height + 20
        }
    }

    DropShadow {
        id: shadow
        anchors.fill: background
        radius: 10
        samples: 17
        color: Qt.rgba(0, 0, 0, 0.75)
        source: background
        cached: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: background
        acceptedButtons: Qt.RightButton
        onClicked: {
            popupMenu.popup()
        }
    }

    ListView {
        id: resultList
        visible: false

        anchors {
            top: background.top
            bottom: inputText.top
            left: background.left
            right: background.right
        }
        clip: true
        model: Mangonel.apps
        orientation: Qt.Horizontal
        highlightMoveDuration: 50
        preferredHighlightBegin: width/2 - itemWidth/2
        preferredHighlightEnd: width/2 +  itemWidth/2
        highlightRangeMode: ListView.StrictlyEnforceRange
        property real itemWidth: count > 1 ? width / 2 : width

        delegate: Item {
            width: resultList.itemWidth
            height: resultList.height

            function launch() {
                modelData.launch(modelData.program)
            }

            property string type: modelData.type
            property string completion: modelData.completion

            opacity: ListView.view.currentIndex === index ? 1 : 0.2
            Behavior on opacity { NumberAnimation { duration: 200 } }

            Image {
                anchors {
                    top: parent.top
                    topMargin: 10
                    horizontalCenter: parent.horizontalCenter
                }

                source: "image://icon/" + modelData.icon
                sourceSize.width: parent.width
                sourceSize.height: parent.height - nameText.height - 20
            }

            Text {
                id: nameText
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 20
                    left: parent.left
                    right: parent.right
                }
                property string name: modelData.name
                onNameChanged: {
                    var index = name.toLowerCase().indexOf(inputText.text.toLowerCase())
                    if (index === -1) {
                        text = name
                        return
                    }

                    text = name.substring(0, index)
                    text += "<b>"
                    text += name.substring(index, index + inputText.text.length)
                    text += "</b>"
                    text += name.substring(index + inputText.text.length)
                }
                color: "white"

                font.pointSize: 20
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: modelData.name
                wrapMode: Text.WordWrap
            }
        }
    }

    LinearGradient {
        id: mask
        anchors.fill: resultList

        gradient: Gradient {
            GradientStop { position: 0.1; color: Qt.rgba(1, 1, 1, 1) }
            GradientStop { position: 0.3; color: Qt.rgba(0, 0, 0, 0) }
            GradientStop { position: 0.7; color: Qt.rgba(0, 0, 0, 0) }
            GradientStop { position: 0.9; color: Qt.rgba(1, 1, 1, 1) }
        }

        start: Qt.point(0, 0)
        end: Qt.point(resultList.width, 0)
        opacity: 0.5
        visible: false
    }

    MaskedBlur {
        anchors.fill: resultList
        source: resultList
        maskSource: mask
        radius: 16
        samples: 24
    }

    Text {
        anchors {
            left: background.left
            leftMargin: 5
            verticalCenter: inputText.verticalCenter
        }
        text: resultList.currentItem !== null ? resultList.currentItem.type + ":" : ""
        color: "white"

    }

    TextInput {
        id: inputText
        anchors {
            horizontalCenter: background.horizontalCenter
            bottom: background.bottom
            bottomMargin: 10
        }

        property var history: Mangonel.history
        property int historyIndex: -1
        property string preHistoryText: ""

        onHistoryIndexChanged: {
            if (historyIndex < 0) {
                text = preHistoryText
                return
            }
            if (historyIndex >= history.length ) return

            text = history[historyIndex]
        }

        Keys.onDownPressed: {
            if (historyIndex >= 0) {
                historyIndex--
            }
        }

        Keys.onUpPressed: {
            if (historyIndex < 0) { // preserve what is written now
                preHistoryText = text
            }

            if (historyIndex <= history.length - 1) {
                historyIndex++
            }
        }

        color: "white"
        focus: true
        font.pointSize: 15
        font.bold: true
        onTextChanged: Mangonel.getApp(text)

        Keys.onEscapePressed: window.visible = false
        Keys.onLeftPressed: {
            if (resultList.currentIndex > 0) {
                resultList.currentIndex--
            }
            event.accepted = true
        }
        Keys.onRightPressed: {
            if (resultList.currentIndex < resultList.count - 1) {
                resultList.currentIndex++
            }
            event.accepted = true
        }
        Keys.onTabPressed: {
            event.accepted = true
            if (resultList.currentItem === null) {
                return
            }
            text = resultList.currentItem.completion
        }

        onAccepted: {
            if (resultList.currentItem === null) {
                return
            }
            Mangonel.addToHistory(text)
            resultList.currentItem.launch()
            window.visible = false
        }
    }

    MouseArea {
        anchors {
            left: background.left
            right: background.right
            bottom: background.bottom
        }

        acceptedButtons: "MiddleButton"
        onClicked: inputText.text += Mangonel.selectionClipboardContent()
        height: bottomBackground.height
    }

    Menu {
        id: popupMenu
        title: "Mangonel"
        MenuItem {
            text: qsTr("Configure &shortcuts")
            iconName: "configure-shortcuts"
            onTriggered: Mangonel.showConfig()
            shortcut: StandardKey.Preferences
        }
        MenuItem {
            text: qsTr("Configure &notifications")
            iconName: "preferences-desktop-notifications"
            onTriggered: Mangonel.configureNotifications()
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("&Quit")
            iconName: "application-exit"
            onTriggered: Qt.quit()
            shortcut: StandardKey.Quit
        }
    }
}
