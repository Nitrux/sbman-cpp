import QtQuick
import QtQuick.Controls 2.15 as QQC
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ApplicationWindow {
    id: root

    title: i18n("Secure Boot Manager")
    color: "transparent"
    background: null

    Maui.WindowBlur {
        view: root
        geometry: Qt.rect(0, 0, root.width, root.height)
        windowRadius: Maui.Style.radiusV
        enabled: true
    }

    Rectangle {
        anchors.fill: parent
        color: Maui.Theme.backgroundColor
        opacity: 0.76
        radius: Maui.Style.radiusV
        border.color: Qt.rgba(1, 1, 1, 0)
        border.width: 1
    }

    Maui.Page {
        anchors.fill: parent
        flickable: scrollColumn.flickable
        background: null
        headerMargins: Maui.Style.contentMargins
        headBar.preferredHeight: Maui.Style.rowHeightAlt
        headBar.topPadding: Maui.Style.defaultPadding
        headBar.bottomPadding: Maui.Style.defaultPadding
        footBar.preferredHeight: Maui.Style.rowHeightAlt
        footBar.topPadding: Maui.Style.defaultPadding
        footBar.bottomPadding: Maui.Style.defaultPadding

        Component.onCompleted: secureBootManager.refresh()

        headBar.leftContent: [
            QQC.ToolButton {
                display: QQC.AbstractButton.IconOnly
                icon.name: "view-refresh"
                enabled: !secureBootManager.running
                onClicked: secureBootManager.refresh()
            }
        ]

        headBar.rightContent: [
            Maui.ToolButtonMenu {
                icon.name: "overflow-menu"

                QQC.MenuItem {
                    text: i18n("About")
                    icon.name: "documentinfo"
                    onTriggered: Maui.App.aboutDialog()
                }
            }
        ]

        Maui.ScrollColumn {
            id: scrollColumn
            anchors.fill: parent
            spacing: Maui.Style.space.big

            Maui.SectionHeader {
                Layout.fillWidth: true
                text1: i18n("Secure Boot")
                text2: i18n("Enroll the Nitrux certificates required by the installed boot packages.")
                label2.wrapMode: Text.Wrap
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: certificatesLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: certificatesLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: i18n("Certificates")
                        text2: secureBootManager.certificates.length === 1
                               ? i18n("The certificate must be approved in MOK Manager after the next reboot.")
                               : i18n("Both certificates must be approved in MOK Manager after the next reboot.")
                        label2.wrapMode: Text.Wrap
                    }

                    Repeater {
                        model: secureBootManager.certificates

                        delegate: Maui.FlexSectionItem {
                            required property var modelData

                            Layout.fillWidth: true
                            flat: true
                            label1.text: modelData.name
                            label2.text: modelData.path
                            label2.elide: Text.ElideMiddle
                            enabled: modelData.available
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: statusLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: statusLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: i18n("System Status")
                        text2: i18n("Check the boot environment and certificate availability.")
                        label2.wrapMode: Text.Wrap
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: i18n("Enrollment")
                        label2.text: secureBootManager.status
                        label2.wrapMode: Text.Wrap
                        iconSource: secureBootManager.ready ? "emblem-default" : "emblem-error"
                    }

                    Maui.FlexSectionItem {
                        Layout.fillWidth: true
                        flat: true
                        label1.text: i18n("Secure Boot")
                        label2.text: secureBootManager.secureBootEnabled
                                      ? i18n("Enabled")
                                      : i18n("Disabled")
                        iconSource: secureBootManager.secureBootEnabled
                                    ? "emblem-default"
                                    : "emblem-error"
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
                implicitHeight: enrollmentLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout {
                    id: enrollmentLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader {
                        Layout.fillWidth: true
                        text1: i18n("Certificate Enrollment")
                        text2: secureBootManager.certificates.length === 1
                               ? i18n("Queue the certificate for approval in MOK Manager after reboot.")
                               : i18n("Queue the certificates for approval in MOK Manager after reboot.")
                        label2.wrapMode: Text.Wrap
                    }

                    QQC.Button {
                        Layout.fillWidth: true
                        text: secureBootManager.running
                              ? i18n("Requesting authorization…")
                              : i18n("Enroll certificates")
                        enabled: secureBootManager.ready
                                  && !secureBootManager.certificatesEnrolled
                                  && !secureBootManager.running
                        Maui.Controls.status: enabled
                                                  ? Maui.Controls.Positive
                                                  : Maui.Controls.Normal
                        onClicked: secureBootManager.importCertificates()
                    }
                }
            }
        }
    }
}
