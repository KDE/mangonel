import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import org.kde 1.0
import QtGraphicalEffects 1.0

Window {
    id: window
    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    color: "transparent"

    height: 650

    width:  300;//Math.max(window.screen.width / 4, 300)
    y: window.screen.height / 2 - height / 2
    x: window.screen.width / 2 - width / 2
    visible :false

    onVisibleChanged:{
        inputText.text = ""

        if (visible) {
            requestActivate()

            var desktopWidth = window.screen.width
            window.width = Math.min(desktopWidth / 1.5, window.height * 2)
        }

        inputText.preHistoryText = ""
        inputText.historyIndex = -1
    }

    onActiveChanged: if (!active) visible = false
    onActiveFocusItemChanged: if (!activeFocusItem) visible = false

    Connections {
        target: Mangonel
        onTriggered: window.visible = true
    }

    Rectangle {
        id: background
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 400 + historyList.height

        Behavior on height { NumberAnimation { duration: 50 } }

        color: Qt.rgba(0, 0, 0, 0.75)
        radius: 10
        border.width: 5
        border.color: "black"

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
            left: background.left
            right: background.right
        }
        height: 350
        clip: true
        orientation: Qt.Horizontal
        highlightMoveDuration: 100
        preferredHighlightBegin: width/2 - itemWidth/2
        preferredHighlightEnd: width/2 +  itemWidth/2
        highlightRangeMode: ListView.StrictlyEnforceRange

        property real itemWidth: height

        delegate: Item {
            width: resultList.itemWidth
            height: resultList.height

            function launch() {
                if (!modelData) {
                    return
                }

                Mangonel.launch(modelData)
            }

            property string type: modelData.type
            property string completion: modelData.completion

            opacity: ListView.view.currentIndex === index ? 1 : 0.2
            Behavior on opacity { NumberAnimation { duration: 100 } }

            Image {
                id: icon
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
                property string name: modelData.completion
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
                text: modelData.completion
                wrapMode: Text.WordWrap
            }
        }
    }

    LinearGradient {
        id: mask
        anchors.fill: resultList
        property real centerLeft: (width / 2 - resultList.itemWidth / 2) / width
        property real centerRight: (width / 2 + resultList.itemWidth / 2) / width

        gradient: Gradient {
            GradientStop { position: 0.1; color: Qt.rgba(1, 1, 1, 1) }
            GradientStop { position: mask.centerLeft; color: Qt.rgba(0, 0, 0, 0) }
            GradientStop { position: mask.centerRight; color: Qt.rgba(0, 0, 0, 0) }
            GradientStop { position: 0.9; color: Qt.rgba(1, 1, 1, 1) }
        }

        start: Qt.point(0, 0)
        end: Qt.point(resultList.width, 0)
        opacity: 0.5
        visible: false
    }

    MaskedBlur {
        anchors.centerIn: resultList
        width: parent.visible ? resultList.width : 0
        height: parent.visible ? resultList.height : 0
        source: resultList
        maskSource: mask
        radius: parent.visible ? 8 : 0
        samples: 24
        Behavior on height { NumberAnimation { duration: 10 } }
        Behavior on width { NumberAnimation { duration: 10 } }
    }

    Text {
        anchors {
            left: background.left
            leftMargin: 15
            verticalCenter: inputText.verticalCenter
        }
        text: (resultList.count > 0 && resultList.currentItem !== null) ? resultList.currentItem.type : ""
        color: "white"
    }

    ListView {
        id: historyList
        anchors {
            bottom: inputText.top
            bottomMargin: 15
        }
        x: inputText.x + inputText.cursorRectangle.x - inputText.width
        height: inputText.historyIndex >= 0 ? 190 : 0
        interactive: false
        highlightFollowsCurrentItem: true
        currentIndex: inputText.historyIndex

        model: height ? Mangonel.history : 0
        verticalLayoutDirection: ListView.BottomToTop

        delegate: Text { text: modelData; color: "white"; font.bold: index == inputText.historyIndex; opacity: font.bold ? 1 : 0.4 }
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
        onTextChanged: resultList.model = Mangonel.setQuery(text)

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
