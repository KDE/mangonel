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

#include "Applications.h"

#include <KServiceTypeTrader>
#include <QDebug>
#include <KLocalizedString>
#include <QProcess>
#include <QSettings>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDateTime>

Applications::Applications(QObject *parent) :
    Provider(parent)
{
}

Applications::~Applications()
{
}

ProviderResult *Applications::createApp(const KService::Ptr &service)
{
    ProviderResult *app = new ProviderResult;
    app->name = service->name();
    app->completion = app->name;
    app->icon = service->icon();
    app->object = this;

    QString exec = service->exec();
    exec.remove(QRegularExpression("\\%[fFuUdDnNickvm]"));

    app->program = exec;
    QFileInfo entryPathInfo(service->entryPath());
    app->priority = QDateTime::currentSecsSinceEpoch();
    if (entryPathInfo.exists()) {
        app->priority = app->priority - entryPathInfo.lastModified().toSecsSinceEpoch();
    } else {
        // KService can't give us the path to anything...
    }
    if (service->isApplication()) {
        app->type = i18n("Run application");
    } else {
        app->type = i18n("Open control module");
    }

    return app;
}

QList<ProviderResult *> Applications::getResults(QString term)
{
    QList<ProviderResult*> list;
    term = term.toHtmlEscaped();
    term.replace('\'', ' ');

    static const QString query = "exist Exec and ( (exist Keywords and '%1' ~subin Keywords) or (exist GenericName and '%1' ~~ GenericName) or (exist Name and '%1' ~~ Name) or ('%1' ~~ Exec) )";

    KService::List services = KServiceTypeTrader::self()->query("Application", query.arg(term));
    services.append(KServiceTypeTrader::self()->query("KCModule", query.arg(term)));

    for (const KService::Ptr &service : services) {
        if (service->noDisplay()) {
            continue;
        }
        
        ProviderResult *app = createApp(service);
        if (app->name.isEmpty()) {
            delete app;
            continue;
        }
        if (service->isApplication()) app->priority *= 1.1;

        list.append(app);
    }
    return list;
}

int Applications::launch(const QString &selected)
{
    
    if (QProcess::startDetached(selected)) {
        return 0;
    } else {
        return 1;
    }
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
