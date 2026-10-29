import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: window
    objectName: "mobileWindow"
    width: 390
    height: 820
    visible: true
    title: "Chess"
    color: "#17241f"
    Material.theme: Material.Dark
    Material.accent: "#dab779"
    Material.primary: "#243a30"
    property bool autoPlaying: false
    property var movingPieces: []
    function isMoving(square) {
        return movingPieces.some(function(piece) { return piece.to === square })
    }
    function animateMove(from, to, rookFrom, rookTo) {
        function piece(start, end) {
            const a = game.humanWhite ? start : 63 - start
            const b = game.humanWhite ? end : 63 - end
            return {from: a, to: end, destination: b, image: game.board[b].image}
        }
        let pieces = [piece(from, to)]
        if (rookFrom >= 0) pieces.push(piece(rookFrom, rookTo))
        movingPieces = pieces
        animationEnd.restart()
    }
    Timer { id: animationEnd; interval: 220; onTriggered: window.movingPieces = [] }

    ScrollView {
        id: scroll
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        ColumnLayout {
            width: scroll.availableWidth
            spacing: 12
            Item { Layout.preferredHeight: 4 }
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label { text: "CHESS"; font.pixelSize: 26; font.bold: true; color: "#f3e9d7" }
                    Label {
                        Layout.fillWidth: true
                        text: game.againstComputer ? "You play " + (game.humanWhite ? "White" : "Black") : "Local two-player"
                        color: "#bdc9bf"
                        wrapMode: Text.WordWrap
                    }
                }
                Button { objectName: "newGameButton"; text: "New game"; onClicked: settings.open() }
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.minimumHeight: 48
                Layout.maximumHeight: 48
                text: game.status
                font.pixelSize: 19
                font.bold: true
                color: "#f3e9d7"
                wrapMode: Text.WordWrap
                Accessible.role: Accessible.StaticText
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                BusyIndicator { running: game.thinking; opacity: running ? 1 : 0; Layout.minimumWidth: 24; Layout.maximumWidth: 24; Layout.minimumHeight: 24; Layout.maximumHeight: 24 }
                Label { Layout.fillWidth: true; text: game.material; color: "#bdc9bf" }
                Label { text: (game.humanWhite ? "Black" : "White") + " ↑"; color: "#bdc9bf" }
            }
            Item {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                Layout.preferredHeight: Math.min(width, 540)
                Grid {
                    id: board
                    objectName: "boardGrid"
                    anchors.centerIn: parent
                    width: Math.min(parent.width, 540)
                    height: width
                    rows: 8
                    columns: 8
                    Repeater {
                        model: game.board
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            objectName: "square" + modelData.index
                            width: board.width / 8
                            height: width
                            color: modelData.selected ? "#e4b959" : modelData.check ? "#d75f55"
                                   : (Math.floor(index / 8) + index % 8) % 2 ? "#79927a" : "#e4ddc8"
                            Accessible.role: Accessible.Button
                            Accessible.name: modelData.coordinate + ", " + (modelData.name || "empty")
                            Accessible.onPressAction: { if (!window.movingPieces.length) game.selectSquare(modelData.index) }
                            Image {
                                anchors.centerIn: parent
                                width: parent.width * 0.86
                                height: width
                                visible: !window.isMoving(modelData.index)
                                source: modelData.image
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                            Rectangle {
                                visible: modelData.legal
                                anchors.centerIn: parent
                                width: parent.width * (modelData.capture ? 0.88 : 0.24)
                                height: width
                                radius: width / 2
                                color: modelData.capture ? "transparent" : "#8831463c"
                                border.width: modelData.capture ? 3 : 0
                                border.color: "#a12e2e"
                            }
                            Text {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: 2
                                visible: index % 8 === 0
                                text: modelData.coordinate.slice(1)
                                font.pixelSize: 10
                                color: "#223b30"
                            }
                            Text {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.margins: 2
                                visible: index >= 56
                                text: modelData.coordinate.slice(0, 1)
                                font.pixelSize: 10
                                color: "#223b30"
                            }
                            MouseArea {
                                anchors.fill: parent
                                enabled: !window.movingPieces.length && !game.thinking && !game.replaying && !game.promotionPending && !game.gameOver
                                onClicked: game.selectSquare(modelData.index)
                            }
                        }
                    }
                }
                Item {
                    anchors.fill: board
                    Repeater {
                        model: window.movingPieces
                        delegate: Image {
                            required property var modelData
                            id: movingPiece
                            objectName: "movingPiece"
                            width: board.width / 8 * 0.86
                            height: width
                            source: modelData.image
                            fillMode: Image.PreserveAspectFit
                            x: (modelData.destination % 8 + 0.07) * board.width / 8
                            y: (Math.floor(modelData.destination / 8) + 0.07) * board.width / 8
                            ParallelAnimation {
                                running: true
                                NumberAnimation { target: movingPiece; property: "x"; from: (modelData.from % 8 + 0.07) * board.width / 8; to: (modelData.destination % 8 + 0.07) * board.width / 8; duration: 200; easing.type: Easing.InOutQuad }
                                NumberAnimation { target: movingPiece; property: "y"; from: (Math.floor(modelData.from / 8) + 0.07) * board.width / 8; to: (Math.floor(modelData.destination / 8) + 0.07) * board.width / 8; duration: 200; easing.type: Easing.InOutQuad }
                            }
                        }
                    }
                }
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: game.replaying ? "Recorded game · " + (game.humanWhite ? "White" : "Black") + " at the bottom"
                                     : "Tap a piece, then a highlighted square to move."
                color: "#bdc9bf"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
            Button {
                Layout.alignment: Qt.AlignHCenter
                visible: game.gameOver && !game.replaying && game.replayLength > 0
                text: "Replay game"
                onClicked: game.startReplay()
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                visible: game.replaying
                spacing: 2
                ToolButton { text: "|‹"; Accessible.name: "First move"; onClicked: { window.autoPlaying = false; game.seekReplay(0) } }
                ToolButton { text: "‹"; Accessible.name: "Previous move"; enabled: game.replayIndex > 0; onClicked: { window.autoPlaying = false; game.seekReplay(game.replayIndex - 1) } }
                Button {
                    text: window.autoPlaying ? "Pause" : "Play"
                    onClicked: {
                        if (!window.autoPlaying && game.replayIndex === game.replayLength) game.seekReplay(0)
                        window.autoPlaying = !window.autoPlaying
                    }
                }
                ToolButton { text: "›"; Accessible.name: "Next move"; enabled: game.replayIndex < game.replayLength; onClicked: { window.autoPlaying = false; game.seekReplay(game.replayIndex + 1) } }
                ToolButton { text: "›|"; Accessible.name: "Last move"; onClicked: { window.autoPlaying = false; game.seekReplay(game.replayLength) } }
            }
            Button {
                Layout.alignment: Qt.AlignHCenter
                visible: game.replaying
                text: "Return to finished game"
                onClicked: game.exitReplay()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                visible: game.saveError.length > 0
                text: game.saveError
                color: "#ffc0ad"
                wrapMode: Text.WordWrap
            }
            Item { Layout.preferredHeight: 12 }
        }
    }

    Timer {
        interval: 900
        repeat: true
        running: window.autoPlaying && game.replaying
        onTriggered: {
            if (game.replayIndex < game.replayLength) game.seekReplay(game.replayIndex + 1)
            if (game.replayIndex === game.replayLength) window.autoPlaying = false
        }
    }

    Dialog {
        id: settings
        objectName: "newGameSettings"
        anchors.centerIn: parent
        width: Math.min(window.width - 24, 360)
        modal: true
        title: "New game"
        standardButtons: Dialog.Ok | Dialog.Cancel
        onOpened: {
            opponent.currentIndex = game.againstComputer ? 0 : 1
            side.currentIndex = game.humanWhite ? 0 : 1
            difficulty.currentIndex = game.difficulty
        }
        onAccepted: { window.movingPieces = []; game.newGame(side.currentIndex === 0, opponent.currentIndex === 0, difficulty.currentIndex) }
        ColumnLayout {
            width: parent.width
            Label { text: "Opponent" }
            ComboBox { id: opponent; Layout.fillWidth: true; model: ["Computer", "Local two-player"]; Accessible.name: "Opponent" }
            Label { text: "Your side" }
            ComboBox { id: side; Layout.fillWidth: true; model: ["White", "Black"]; Accessible.name: "Your side" }
            Label { text: "Difficulty" }
            ComboBox { id: difficulty; Layout.fillWidth: true; model: ["Easy", "Medium", "Hard"]; enabled: opponent.currentIndex === 0; Accessible.name: "Difficulty" }
            Label { Layout.fillWidth: true; text: "Starting a new game replaces the saved game."; wrapMode: Text.WordWrap; color: "#bdc9bf" }
        }
    }

    Dialog {
        id: promotion
        anchors.centerIn: parent
        width: Math.min(window.width - 24, 350)
        modal: true
        title: "Promote your pawn"
        closePolicy: Popup.NoAutoClose
        ColumnLayout {
            width: parent.width
            Repeater {
                model: [{name: "Queen", kind: 5}, {name: "Rook", kind: 4}, {name: "Bishop", kind: 3}, {name: "Knight", kind: 2}]
                Button { required property var modelData; Layout.fillWidth: true; text: modelData.name; onClicked: game.promote(modelData.kind) }
            }
            Button { Layout.fillWidth: true; text: "Cancel"; onClicked: game.cancelPromotion() }
        }
    }

    Connections {
        target: game
        function onMoveMade(from, to, rookFrom, rookTo) { window.animateMove(from, to, rookFrom, rookTo) }
        function onStateChanged() {
            if (game.promotionPending) promotion.open()
            else promotion.close()
            if (!game.replaying) window.autoPlaying = false
        }
    }
}
