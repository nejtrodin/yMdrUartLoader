import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0
import com.mycompany.loader 1.0
import Qt.labs.settings 1.0

Window {
    visible: true
    id: mainWindow
    width: 600
    height: 700
    color: Material.background

    property color buttonActiveColor: Material.accent
    property color buttonSuccessColor: Material.color(Material.Green)
    property color buttonErrorColor: Material.color(Material.Red)
    property color buttonDefaultColor: Material.theme == Material.Dark ?
                                           Qt.lighter(Material.background, 1.5) :
                                           Qt.darker(Material.background, 1.2)

    Timer {
        id: scanDeviceTimer
        interval: 1000; running: true; repeat: true
        onTriggered: mainWindow.checkPorts()
    }

    Settings {
        id: settings
        property string codePath: ""
        property int baudIndex: 0
        property bool getRevisionFlag: false
        property bool writeCodeFlag: true
        property bool verifyCodeFlag: true
        property bool runCodeFlag: true
        property string portName: ""
        property int mcuTypeIndex: 0
    }

    Component.onDestruction: {
        if (pathEdit.text.length > 0)
            settings.codePath = pathEdit.text
        settings.baudIndex = portBaudComboBox.currentIndex
        settings.getRevisionFlag = getRevCheckBox.checked
        settings.writeCodeFlag = writeCodeChechBox.checked
        settings.verifyCodeFlag = verifyCheckBox.checked
        settings.runCodeFlag = runCodeCheckBox.checked
        var portName = portNameComboBox.currentText
        if (portName !== "")
            settings.portName = portName
        settings.mcuTypeIndex = mcuTypeComboBox.currentIndex
    }

    property bool startFlag: false
    Connections {
        target: loader
        onSignalRevision: {
            if (ok) {
                getRevButton.text = qsTr("Get Revision") + " [" + revision + "]"
                getRevButton.backColor = buttonSuccessColor
                if (startFlag) {
                    writeLoaderButton.backColor = buttonActiveColor
                    loader.writeLoader()
                }
            }
            else {
                getRevButton.text = qsTr("Get Revision")
                getRevButton.backColor = buttonErrorColor
                if (startFlag) {
                    startFlag = false
                    enableGui()
                }
            }
        }
        onSignalLoaderWrited: {
            if (ok) {
                writeLoaderButton.backColor = buttonSuccessColor
                if (startFlag) {
                    if (writeCodeChechBox.checked) {
                        eraseButton.backColor = buttonActiveColor
                        loader.eraseFlash()
                    }
                    else if (verifyCheckBox.checked) {
                        verifyButton.backColor = buttonActiveColor
                        loader.verifyFlash()
                    }
                    else if (runCodeCheckBox.checked) {
                        runCodeButton.backColor = buttonActiveColor
                        loader.runMcu()
                    }
                    else {
                        startFlag = false
                        enableGui()
                    }
                }
            }
            else {
                writeLoaderButton.backColor = buttonErrorColor
                if (startFlag) {
                    startFlag = false
                    enableGui()
                }
            }
        }
        onSignalFlashErased: {
            if (ok) {
                eraseButton.backColor = buttonSuccessColor
                if (startFlag) {
                    writeCodeButton.backColor = buttonActiveColor
                    loader.writeFlash(pathEdit.text)
                }
            }
            else {
                eraseButton.backColor = buttonErrorColor
                if (startFlag) {
                    startFlag = false
                    enableGui()
                }
            }
        }
        onSignalFlashWrited: {
            if (ok) {
                writeCodeButton.backColor = buttonSuccessColor
                if (startFlag) {
                    if (verifyCheckBox.checked) {
                        verifyButton.backColor = buttonActiveColor
                        loader.verifyFlash(pathEdit.text)
                    }
                    else if (runCodeCheckBox.checked) {
                        runCodeButton.backColor = buttonActiveColor
                        loader.runMcu()
                    }
                    else {
                        startFlag = false
                        enableGui()
                    }
                }
            }
            else {
                writeCodeButton.backColor = buttonErrorColor
                if (startFlag) {
                    startFlag = false
                    enableGui()
                }
            }
        }
        onSignalFlashVerified: {
            if (ok) {
                verifyButton.backColor = buttonSuccessColor
                if (startFlag) {
                    if (runCodeCheckBox.checked) {
                        runCodeButton.backColor = buttonActiveColor
                        loader.runMcu()
                    }
                    else {
                        startFlag = false
                        enableGui()
                    }
                }
            }
            else {
                verifyButton.backColor = buttonErrorColor
                if (startFlag) {
                    startFlag = false
                    enableGui()
                }
            }
        }
        onSignalMcuRuned: {
            if (ok) {
                runCodeButton.backColor = buttonSuccessColor
            }
            else {
                runCodeButton.backColor = buttonErrorColor
            }
            if (startFlag) {
                startFlag = false
                enableGui()
            }
        }
        onSignalWriteLog: {
            logTextArea.append(line)
        }
    }

    property bool firstCheckPortFlag: true
    function checkPorts() {
        var list = loader.portList
        portNameComboBox.model = list
        if ((list.length > 0) && (firstCheckPortFlag)) {
            for (var i = 0; i < list.length; i++) {
                if (list[i] === settings.portName) {
                    portNameComboBox.currentIndex = i
                    break
                }
            }
            firstCheckPortFlag = false
        }
        if (list.length === 0) {
            firstCheckPortFlag = true
        }
    }

    function startButtonClicked() {
        startFlag = true
        // disable all gui elements
        getRevCheckBox.enabled = false
        writeCodeChechBox.enabled = false
        verifyCheckBox.enabled = false
        runCodeCheckBox.enabled = false
        portNameComboBox.enabled = false
        portBaudComboBox.enabled = false
        mainWindow.clearButtonColor()
        loader.setSettings(mcuTypeComboBox.currentText,
                           portNameComboBox.currentText,
                           portBaudItems.get(portBaudComboBox.currentIndex).value)
        if (getRevCheckBox.checked) {
            getRevButton.backColor = buttonActiveColor
            loader.getRevision()
            return
        }
        writeLoaderButton.backColor = buttonActiveColor
        loader.writeLoader()
    }

    function clearButtonColor() {
        getRevButton.backColor = buttonDefaultColor
        writeLoaderButton.backColor = buttonDefaultColor
        eraseButton.backColor = buttonDefaultColor
        writeCodeButton.backColor = buttonDefaultColor
        verifyButton.backColor = buttonDefaultColor
        runCodeButton.backColor = buttonDefaultColor
    }

    function enableGui() {
        getRevCheckBox.enabled = true
        writeLoaderButton.enabled = true
        writeCodeChechBox.enabled = true
        verifyCheckBox.enabled = true
        runCodeCheckBox.enabled = true
        portNameComboBox.enabled = true
        portBaudComboBox.enabled = true
    }


    ColumnLayout {  // main Layout
        anchors.margins: 5
        RowLayout {
            Label {
                text: "McuType: "
            }
            ComboBox {
                id: mcuTypeComboBox
                model: ["1986ВЕ1Т", "1986ВЕ9x"]
                Layout.fillWidth: true
                currentIndex: settings.mcuTypeIndex
            }
            spacing: 10
        }

        RowLayout {  // path Layout
            Label {
                text: "Path to firmware: "
            }
            TextField {
                id: pathEdit
                text: settings.codePath
                selectByMouse: true
                Layout.fillWidth: true
            }
            Button {
                text: "..."
                onClicked: openDialog.open()
            }
            spacing: 10
        }

        RowLayout {  // port, baud Layout
            Label {
                text: "Port: "
            }
            ComboBox {
                id: portNameComboBox
                implicitWidth: 150
            }
            Label {
                text: "Baud: "
            }
            ComboBox {
                id: portBaudComboBox
                textRole: "text"
                model: ListModel {
                    id: portBaudItems
                    ListElement { text: "9600"; value: 9600 }
                    ListElement { text: "19200"; value: 19200 }
                    ListElement { text: "57600"; value: 57600 }
                    ListElement { text: "115200"; value: 115200 }
                }
                currentIndex: settings.baudIndex
            }
            spacing: 10
        }

        RowLayout {  // get revision Layout
            CheckBox {
                id: getRevCheckBox
                checked: settings.getRevisionFlag
            }
            Button {
                id: getRevButton
                property color backColor: buttonDefaultColor
                text: qsTr("Get Revision")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                           " - Get revision");
                        mainWindow.clearButtonColor()
                        getRevButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.getRevision()
                    }
                }
            }
            spacing: 10
        }

        RowLayout {  // write ram loader
            CheckBox {
                id: writeLoaderBox
                checked: true
                enabled: false
            }
            Button {
                id: writeLoaderButton
                property color backColor: buttonDefaultColor
                text: qsTr("Write Bootloader")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                           " - Write Bootloader");
                        mainWindow.clearButtonColor()
                        writeLoaderButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.writeLoader()
                    }
                }
            }
            spacing: 10
        }

        RowLayout {
            CheckBox {
                id: eraseCheckBox
                checked: settings.writeCodeFlag
                enabled: false
            }
            Button {
                id: eraseButton
                property color backColor: buttonDefaultColor
                text: qsTr("Erase flash")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                                             " - Erase flash");
                        mainWindow.clearButtonColor()
                        eraseButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.eraseFlash()
                    }
                }
            }
            spacing: 10
        }

        RowLayout {
            CheckBox {
                id: writeCodeChechBox
                checked: settings.writeCodeFlag
                onClicked: eraseCheckBox.checked = writeCodeChechBox.checked
            }
            Button {
                id: writeCodeButton
                property color backColor: buttonDefaultColor
                text: qsTr("Write code")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                           " - Write code");
                        if (pathEdit.text.length == 0) {
                            logTextArea.append(qsTr("Path to file is empty"))
                            return
                        }
                        mainWindow.clearButtonColor()
                        writeCodeButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.writeFlash(pathEdit.text)
                    }
                }
            }
            spacing: 10
        }

        RowLayout {
            CheckBox {
                id: verifyCheckBox
                checked: settings.verifyCodeFlag
            }
            Button {
                id: verifyButton
                property color backColor: buttonDefaultColor
                text: qsTr("Verify code")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                           " - Verify code");
                        if (pathEdit.text.length == 0) {
                            logTextArea.append(qsTr("Path to file is empty"))
                            return
                        }
                        mainWindow.clearButtonColor()
                        verifyButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.verifyFlash(pathEdit.text)
                    }
                }
            }
            spacing: 10
        }

        RowLayout {
            CheckBox {
                id: runCodeCheckBox
                checked: settings.runCodeFlag
            }
            Button {
                id: runCodeButton
                property color backColor: buttonDefaultColor
                text: qsTr("Run MCU")
                font.capitalization: Font.MixedCase
                Layout.fillWidth: true
                Material.elevation: 2
                Material.background: backColor

                onClicked: {
                    if (!startFlag) {
                        logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                           " - Run MCU");
                        mainWindow.clearButtonColor()
                        runCodeButton.backColor = buttonActiveColor
                        loader.setSettings(mcuTypeComboBox.currentText,
                                           portNameComboBox.currentText,
                                           portBaudItems.get(portBaudComboBox.currentIndex).value)
                        loader.runMcu()
                    }
                }
            }
            spacing: 10
        }

        Button {
            id: startButton
            property color backColor: buttonDefaultColor
            text: qsTr("Start")
            font.capitalization: Font.MixedCase
            Layout.fillWidth: true
            Material.elevation: 2
            Material.background: backColor

            onClicked: {
                logTextArea.append(Qt.formatDateTime(new Date(), "* hh:mm:ss") +
                                   " - Start");
                if (pathEdit.text.length == 0 &&
                        (writeCodeChechBox.checked || verifyCheckBox.checked))
                {
                    logTextArea.append(qsTr("Path to file is empty"))
                    return
                }
                mainWindow.startButtonClicked()
            }
        }

        RowLayout {
            ScrollView {
                TextArea {
                    id: logTextArea
                    readOnly: true
                    selectByMouse: true
                    selectByKeyboard: true
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2
            }

            ColumnLayout {
                ToolButton {
                    id: clearLogButton
                    icon.source: "qrc:/images/delete.svg"
                    onClicked: logTextArea.clear()
                }

                ToolButton {
                    id: saveLogButton
                    icon.source: "qrc:/images/save.svg"
                    onClicked: saveLogDialog.open()
                }
            }
        }

        anchors.fill: parent
    }  // ColumnLayout - main Layout


    // Native File Dialogs
    FileDialog {
        id: openDialog
        folder: shortcuts.home
        title: qsTr("Please choose a hex file")
        fileMode: FileDialog.OpenFile
        nameFilters: [ qsTr("Hex files (*.hex)"), qsTr("All files (*)")]

        onAccepted: {
            if (Qt.platform.os == "windows")
                pathEdit.text = openDialog.file.toString().replace("file:///", "")
            else
                pathEdit.text = openDialog.file.toString().replace("file://", "")
        }
    }

    FileDialog {
        id: saveLogDialog
        title: qsTr("Please choose a log file")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "log"
        nameFilters: [qsTr("Log files (*.log)"), qsTr("All files (*.*)")]

        onAccepted: {
            if (saveLogDialog.file != "") {
                if (Qt.platform.os == "windows")
                    fileIoHelper.writeTextFile(saveLogDialog.file.toString().replace("file:///", ""),
                                               logTextArea.text);
                else
                    fileIoHelper.writeTextFile(saveLogDialog.file.toString().replace("file://", ""),
                                               logTextArea.text);
            }
        }
    }
}  // Window
