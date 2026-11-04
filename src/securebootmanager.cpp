// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

#include "securebootmanager.h"

#include <QFileDevice>
#include <QFileInfo>
#include <QSysInfo>

namespace
{
constexpr auto kMokutilPath = "/usr/bin/mokutil";
constexpr auto kPkexecPath = "/usr/bin/pkexec";
constexpr qsizetype kMaximumProcessOutputSize = 4096;

QString trustedSystemFilePath(const QString &path, bool executable)
{
    const QFileInfo fileInfo(path);
    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (canonicalPath.isEmpty())
        return {};

    const QFileInfo canonicalFileInfo(canonicalPath);
    const auto permissions = canonicalFileInfo.permissions();
    const bool writableByNonOwner = permissions.testFlag(QFileDevice::WriteGroup)
        || permissions.testFlag(QFileDevice::WriteOther);

    if (!canonicalFileInfo.isFile() || canonicalFileInfo.ownerId() != 0 || writableByNonOwner)
        return {};

    if (executable && !canonicalFileInfo.isExecutable())
        return {};

    return canonicalPath;
}

bool trustedCertificateFile(const QString &path)
{
    return !trustedSystemFilePath(path, false).isEmpty();
}

const QList<SecureBootManager::Certificate> certificateDefinitions()
{
    const SecureBootManager::Certificate kernelCertificate{
        QStringLiteral("Nitrux kernel Secure Boot certificate"),
        QStringLiteral("/usr/share/nitrux/secure-boot/kernel.cer")};

    if (QSysInfo::currentCpuArchitecture() == QStringLiteral("arm64"))
        return {kernelCertificate};

    return {
        {QStringLiteral("Nitrux GRUB Secure Boot certificate"),
         QStringLiteral("/usr/share/grub/nitrux-secure-boot.cer")},
        kernelCertificate,
    };
}
}

SecureBootManager::SecureBootManager(QObject *parent)
    : QObject(parent)
    , m_certificates(certificateDefinitions())
{
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (!m_running)
            return;

        const QString message = error == QProcess::FailedToStart
            ? QStringLiteral("Could not start pkexec or mokutil.")
            : QStringLiteral("The Secure Boot enrollment process stopped unexpectedly.");
        finishEnrollment(false, message);
    });

    connect(&m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (!m_running)
                    return;

                const QString output = QString::fromLocal8Bit(m_process.readAll().left(kMaximumProcessOutputSize)).trimmed();
                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    const QString approvalMessage = m_certificates.size() == 1
                        ? QStringLiteral("The certificate is queued. Reboot and approve the request in MOK Manager.")
                        : QStringLiteral("The certificates are queued. Reboot and approve both requests in MOK Manager.");
                    finishEnrollment(
                        true,
                        approvalMessage);
                    return;
                }

                const QString detail = output.isEmpty() ? QString() : QStringLiteral("\n\n%1").arg(output);
                finishEnrollment(false, QStringLiteral("mokutil could not queue the certificates.%1").arg(detail));
            });

    refresh();
}

QVariantList SecureBootManager::certificates() const
{
    QVariantList result;
    for (const auto &certificate : m_certificates) {
        result.append(QVariantMap{
            {QStringLiteral("name"), certificate.name},
            {QStringLiteral("path"), certificate.path},
            {QStringLiteral("available"), trustedCertificateFile(certificate.path)},
        });
    }
    return result;
}

QString SecureBootManager::status() const
{
    return m_status;
}

bool SecureBootManager::ready() const
{
    return m_ready;
}

bool SecureBootManager::running() const
{
    return m_running;
}

bool SecureBootManager::uefiAvailable() const
{
    return m_uefiAvailable;
}

bool SecureBootManager::mokutilAvailable() const
{
    return m_mokutilAvailable;
}

bool SecureBootManager::secureBootEnabled() const
{
    return m_secureBootEnabled;
}

bool SecureBootManager::certificatesEnrolled() const
{
    return m_certificatesEnrolled;
}

void SecureBootManager::refresh()
{
    m_uefiAvailable = QFileInfo::exists(QStringLiteral("/sys/firmware/efi"));
    m_mokutilAvailable = !trustedSystemFilePath(QString::fromLatin1(kMokutilPath), true).isEmpty();
    m_secureBootEnabled = m_mokutilAvailable && querySecureBootEnabled();

    bool certificatesAvailable = true;
    for (const auto &certificate : m_certificates) {
        certificatesAvailable = certificatesAvailable && trustedCertificateFile(certificate.path);
    }

    m_certificatesEnrolled = certificatesAvailable;
    if (m_certificatesEnrolled) {
        for (const auto &certificate : m_certificates) {
            if (!certificateEnrolled(certificate.path)) {
                m_certificatesEnrolled = false;
                break;
            }
        }
    }

    m_ready = m_uefiAvailable && m_mokutilAvailable && certificatesAvailable;
    if (m_ready) {
        m_status = m_certificatesEnrolled
            ? (m_certificates.size() == 1
                   ? QStringLiteral("The Nitrux kernel certificate is already enrolled or queued.")
                   : QStringLiteral("The Nitrux certificates are already enrolled or queued."))
            : m_secureBootEnabled
            ? (m_certificates.size() == 1
                   ? QStringLiteral("Secure Boot is enabled. The Nitrux kernel certificate is ready to enroll.")
                   : QStringLiteral("Secure Boot is enabled. Both Nitrux certificates are ready to enroll."))
            : (m_certificates.size() == 1
                   ? QStringLiteral("The Nitrux kernel certificate is ready. Secure Boot is currently disabled.")
                   : QStringLiteral("Both Nitrux certificates are ready. Secure Boot is currently disabled."));
    } else {
        m_status = missingRequirements();
    }

    Q_EMIT stateChanged();
}

void SecureBootManager::importCertificates()
{
    if (m_running)
        return;

    refresh();
    if (!m_ready) {
        Q_EMIT enrollmentFinished(false, m_status);
        return;
    }

    if (m_certificatesEnrolled)
        return;

    const QString pkexec = trustedSystemFilePath(QString::fromLatin1(kPkexecPath), true);
    const QString mokutil = trustedSystemFilePath(QString::fromLatin1(kMokutilPath), true);
    if (pkexec.isEmpty() || mokutil.isEmpty()) {
        finishEnrollment(false, QStringLiteral("pkexec is not installed."));
        return;
    }

    QStringList arguments{
        mokutil,
        QStringLiteral("--import"),
        QStringLiteral("--root-pw"),
    };
    for (const auto &certificate : m_certificates) {
        const QString trustedPath = trustedSystemFilePath(certificate.path, false);
        if (trustedPath.isEmpty()) {
            finishEnrollment(false, QStringLiteral("The packaged Secure Boot certificates are not available."));
            return;
        }
        arguments.append(trustedPath);
    }

    // mokutil uses the root password hash for the one-time MOK approval
    // password, allowing the GUI to invoke it without a terminal.
    m_running = true;
    m_status = QStringLiteral("Requesting administrator authorization...");
    Q_EMIT stateChanged();

    m_process.setProgram(pkexec);
    m_process.setArguments(arguments);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    m_process.start();
}

void SecureBootManager::finishEnrollment(bool success, const QString &message)
{
    m_running = false;
    if (success)
        m_certificatesEnrolled = true;
    m_status = message;
    Q_EMIT stateChanged();
    Q_EMIT enrollmentFinished(success, message);
}

bool SecureBootManager::querySecureBootEnabled() const
{
    const QString mokutil = trustedSystemFilePath(QString::fromLatin1(kMokutilPath), true);
    if (mokutil.isEmpty())
        return false;

    QProcess process;
    process.start(mokutil, {QStringLiteral("--sb-state")});
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }

    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    return output.contains(QStringLiteral("SecureBoot enabled"), Qt::CaseInsensitive);
}

bool SecureBootManager::certificateEnrolled(const QString &path) const
{
    const QString mokutil = trustedSystemFilePath(QString::fromLatin1(kMokutilPath), true);
    const QString certificate = trustedSystemFilePath(path, false);
    if (mokutil.isEmpty() || certificate.isEmpty())
        return false;

    QProcess process;
    process.start(mokutil, {QStringLiteral("--mok"), QStringLiteral("--test-key"), certificate});
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

QString SecureBootManager::missingRequirements() const
{
    if (!m_uefiAvailable)
        return QStringLiteral("This system is not booted in UEFI mode.");
    if (!m_mokutilAvailable)
        return QStringLiteral("mokutil is not installed.");

    for (const auto &certificate : m_certificates) {
        if (!trustedCertificateFile(certificate.path))
            return QStringLiteral("The packaged Secure Boot certificates are not available.");
    }

    return QStringLiteral("Secure Boot enrollment is unavailable.");
}
