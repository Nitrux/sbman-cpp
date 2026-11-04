// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

#pragma once

#include <QProcess>
#include <QList>
#include <QString>
#include <QVariantList>

#include <QObject>

class SecureBootManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList certificates READ certificates NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(bool running READ running NOTIFY stateChanged)
    Q_PROPERTY(bool uefiAvailable READ uefiAvailable NOTIFY stateChanged)
    Q_PROPERTY(bool mokutilAvailable READ mokutilAvailable NOTIFY stateChanged)
    Q_PROPERTY(bool secureBootEnabled READ secureBootEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool certificatesEnrolled READ certificatesEnrolled NOTIFY stateChanged)

public:
    struct Certificate
    {
        QString name;
        QString path;
    };

    explicit SecureBootManager(QObject *parent = nullptr);

    QVariantList certificates() const;
    QString status() const;
    bool ready() const;
    bool running() const;
    bool uefiAvailable() const;
    bool mokutilAvailable() const;
    bool secureBootEnabled() const;
    bool certificatesEnrolled() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void importCertificates();

Q_SIGNALS:
    void stateChanged();
    void enrollmentFinished(bool success, const QString &message);

private:
    void finishEnrollment(bool success, const QString &message);
    bool certificateEnrolled(const QString &path) const;
    bool querySecureBootEnabled() const;
    QString missingRequirements() const;

    QList<Certificate> m_certificates;
    QString m_status;
    bool m_ready = false;
    bool m_running = false;
    bool m_uefiAvailable = false;
    bool m_mokutilAvailable = false;
    bool m_secureBootEnabled = false;
    bool m_certificatesEnrolled = false;
    QProcess m_process;
};
