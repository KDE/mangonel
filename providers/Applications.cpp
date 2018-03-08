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
    QSettings settings;
    settings.beginGroup("applications");
    for (const QString &key : settings.childGroups()) {
        settings.beginGroup(key);
        popularity pop;
        pop.count = settings.value("launches").toLongLong();
        pop.lastUse = settings.value("lastUse").toLongLong();
        m_popularities.insert(key, pop);
        settings.endGroup();
    }
}

Applications::~Applications()
{
    storePopularities();
}

Application *Applications::createApp(const KService::Ptr &service)
{
    Application *app = new Application;
    app->name = service->name();
    app->completion = app->name;
    app->icon = service->icon();
    app->object = this;
    app->program = service->exec();
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

QList<Application *> Applications::getResults(QString term)
{
    QList<Application*> list;
    term = term.toHtmlEscaped();
    term.replace('\'', " ");
    QString query = "exist Exec and ( (exist Keywords and '%1' ~subin Keywords) or (exist GenericName and '%1' ~~ GenericName) or (exist Name and '%1' ~~ Name) or ('%1' ~~ Exec) )";
    query = query.arg(term);
    KService::List services = KServiceTypeTrader::self()->query("Application", query);
    services.append(KServiceTypeTrader::self()->query("KCModule", query));
    foreach(const KService::Ptr &service, services) {
        if (service->noDisplay())
            continue;
        
        Application *app = createApp(service);
        if (app->name.isEmpty()) {
            delete app;
            continue;
        }
        
        if (m_popularities.contains(service->exec())) {
            app->priority = QDateTime::currentSecsSinceEpoch() - m_popularities[service->exec()].lastUse;
            app->priority -= (3600 * 360) * m_popularities[service->exec()].count;
        } else {
            qreal modifier = 1;
            if (service->isApplication()) modifier *= 1.1;
            if (app->name.startsWith(term)) modifier *= 1.5;
            if (app->name.startsWith(term, Qt::CaseInsensitive)) modifier *= 1.1;
            if (app->name.contains(term)) modifier *= 1.05;
            if (app->name.contains(term, Qt::CaseInsensitive)) modifier *= 1.01;
            if (!app->name.contains(term, Qt::CaseInsensitive)) modifier /= 2;

            modifier += 1. / app->name.length();
            app->priority /= modifier;
        }

        list.append(app);
    }
    return list;
}

int Applications::launch(QVariant selected)
{
    QString exec = selected.toString();
    exec.remove(QRegularExpression("\\%[fFuUdDnNickvm]"));
    popularity pop;
    if (m_popularities.contains(exec)) {
        pop = m_popularities[exec];
        pop.lastUse = QDateTime::currentSecsSinceEpoch();
        pop.count++;
    } else {
        pop.lastUse = QDateTime::currentSecsSinceEpoch();
        pop.count = 0;
    }
    m_popularities[selected.toString()] = pop;
    
    storePopularities();
    
    if (QProcess::startDetached(exec)) {
        return 0;
    } else {
        return 1;
    }
}

void Applications::storePopularities()
{
    QSettings settings;
    settings.beginGroup("applications");
    for (const QString &key : m_popularities.keys()) {
        settings.beginGroup(key);
        settings.setValue("launches", m_popularities[key].count);
        settings.setValue("lastUse", m_popularities[key].lastUse);
        settings.endGroup();
    }

}


// kate: indent-mode cstyle; space-indent on; indent-width 4; 
