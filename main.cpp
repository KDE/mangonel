/*
 * Copyright 2010-2012 Bart Kroon <bart@tarmack.eu>
 * Copyright 2012, 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Mangonel.h"
#include "IconProvider.h"

#include <QApplication>
#include <KDBusService>
#include <KAboutData>
#include <klocalizedstring.h>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("KDE");
    app.setQuitOnLastWindowClosed(false);

    KAboutData aboutData (
                       QStringLiteral("mangonel"),
                       i18n("Mangonel"),
                       QStringLiteral("1.1"));
    aboutData.setShortDescription(i18n("A simple application launcher by KDE."));
    aboutData.setLicense(KAboutLicense::BSDL);
    aboutData.setHomepage(QByteArray("www.tarmack.eu/mangonel/"));
    aboutData.addAuthor(i18n("Martin Sandsmark"), i18n("Developer"), "martin.sandsmark@kde.org", "http://iskrembilen.com/");
    aboutData.addAuthor(i18n("Bart Kroon"), i18n("Developer, original author"), "", "http://tarmack.eu/");
    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);
    QObject::connect(&service, &KDBusService::activateRequested, Mangonel::instance(), &Mangonel::triggered);

    qmlRegisterSingletonType<Mangonel>("org.kde", 1, 0, "Mangonel", [](QQmlEngine *, QJSEngine*) -> QObject* {
        QQmlEngine::setObjectOwnership(Mangonel::instance(), QQmlEngine::CppOwnership);
        return Mangonel::instance();
    });
    QQuickWindow::setDefaultAlphaBuffer(true);

    QQmlApplicationEngine qmlEngine;
    qmlEngine.addImageProvider("icon", new IconProvider);
    qmlEngine.load(QUrl("qrc:/main.qml"));
    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
