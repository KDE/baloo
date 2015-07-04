/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2015  Pinak Ahuja <pinak.ahuja@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: mainWindow
    title: qsTr("Baloo Monitor")
    width: 640
    height: 480
    visible: true
    SystemPalette { id: myPalette; colorGroup: SystemPalette.Active }

    Rectangle {
        anchors.fill: parent
        color: myPalette.window
        visible: monitor.balooRunning

        Label {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20

            elide: Text.ElideMiddle

            id: url
            text: "Indexing: " + monitor.url
        }

        RowLayout {
            id: progressLayout
            anchors.top: url.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20

            spacing: 20

            ProgressBar {
                id: progress
                Layout.fillWidth: true
                maximumValue: monitor.totalFiles
                value: monitor.filesIndexed
            }

            Button {
                id: toggleButton
                text: monitor.suspendState
                onClicked: monitor.toggleSuspendState()
            }
        }

        Label {
            id: remainingTime
            anchors.top: progressLayout.bottom
            anchors.left: parent.left
            anchors.margins: 20
            text: "Remaining Time: " + monitor.remainingTime
        }
    }

    ColumnLayout {
        visible: !monitor.balooRunning
        anchors.centerIn: parent
        anchors.margins: 20

        spacing: 20

        Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: qsTr("Baloo is not running")
        }

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            id: startBaloo
            text: qsTr("Start Baloo")
            onClicked: monitor.startBaloo()
        }
    }
}
