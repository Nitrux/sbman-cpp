// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

#include "securebootmanager.h"

#include <QGuiApplication>
#include <QIcon>
#include <QDate>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QUrl>

#include <KAboutData>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <MauiKit4/Core/mauiapp.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Nitrux"));
    app.setApplicationName(QStringLiteral("sbman"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("security-high")));

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("sbman"));

    KAboutData about(QStringLiteral("sbman"),
                     i18n("Secure Boot Manager"),
                     QStringLiteral(SBMAN_VERSION_STRING),
                     i18n("Enroll the Nitrux Secure Boot certificates."),
                     KAboutLicense::BSD_3_Clause,
                     i18n("© %1 Made by Nitrux | Built with MauiKit", QString::number(QDate::currentDate().year())),
                     QString::fromUtf8(GIT_BRANCH) + QStringLiteral("/") + QString::fromUtf8(GIT_COMMIT_HASH));
    about.addAuthor(QStringLiteral("Uri Herrera"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage(QStringLiteral("https://nxos.org"));
    about.setProductName(QByteArrayLiteral("nitrux/sbman"));
    about.setOrganizationDomain(QByteArrayLiteral("nxos.org"));
    about.setDesktopFileName(QStringLiteral("org.nxos.sbman"));
    about.setProgramLogo(app.windowIcon());
    KAboutData::setApplicationData(about);

    MauiApp::instance()->setIconName(QStringLiteral("security-high"));

    SecureBootManager manager;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedQmlContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("secureBootManager"), &manager);

    const QUrl url(QStringLiteral("qrc:/qt/qml/app/nxos/sbman/main.qml"));
    QObject::connect(&engine,
                     &QQmlApplicationEngine::objectCreated,
                     &app,
                     [url](QObject *object, const QUrl &objectUrl) {
                         if (!object && objectUrl == url)
                             QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);

    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
