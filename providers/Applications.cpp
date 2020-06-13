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

#include <QDebug>
#include <KLocalizedString>
#include <QProcess>
#include <QSettings>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QDir>

Applications::Applications(QObject *parent) :
    Provider(parent),
    m_fsWatcher(new QFileSystemWatcher(this))
{
    for (const QString &dirPath : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
        m_fsWatcher->addPath(dirPath);
        loadDir(dirPath);
    }
    for (const QString &dirPath : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        m_fsWatcher->addPath(dirPath + QStringLiteral("/kservices5/"));
        loadDir(dirPath + QStringLiteral("/kservices5/")); // kcm files
    }

    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &Applications::onDirectoryChanged);
}

Applications::~Applications()
{
}

ProviderResult *Applications::createApp(const Application &service)
{
    ProviderResult *app = new ProviderResult;
    app->name = service.name;
    app->completion = app->name;
    app->icon = service.icon;
    app->object = this;

    QString exec = service.exec;
    exec.remove(QRegularExpression("\\%[fFuUdDnNickvm]"));

    app->program = exec;
    app->priority = service.lastModified;

    if (service.exec.contains("kcmshell")) { // so sue me
        app->type = i18n("Open control module");
    } else {
        app->type = i18n("Run application");
    }

    return app;
}

QList<ProviderResult *> Applications::getResults(QString term)
{
    QList<ProviderResult*> list;
    term = term.toLower(); // we lowercase the keywords when indexing

    qint64 currentSecsSinceEpoch = QDateTime::currentSecsSinceEpoch();

    for (const Application &application : m_applications) {
        if (!application.keywords.contains(term)) {
            continue;
        }
        ProviderResult *app = createApp(application);
        if (app->name.isEmpty()) {
            delete app;
            continue;
        }
        app->priority = currentSecsSinceEpoch - app->priority;
        //if (service->isApplication()) app->priority *= 1.1;

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

void Applications::onDirectoryChanged(const QString &path)
{
    QMutableHashIterator<QString, Application> it(m_applications);
    while (it.hasNext()) {
        it.next();
        if (it.key().startsWith(path)) {
            it.remove();
        }
    }
    loadDir(path);
}

void Applications::loadDir(const QString &path)
{
    for (const QFileInfo &file : QDir(path).entryInfoList(QStringList("*.desktop"))) {
        Application app = loadDesktopFile(file);
        if (!app.isValid()) {
            continue;
        }
        m_applications[file.absoluteFilePath()] = app;
    }
}

Applications::Application Applications::loadDesktopFile(const QFileInfo &fileInfo)
{
    // Ugliest implementation of .desktop file reading ever
    // Don't remember why I didn't use QSettings

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << fileInfo.fileName();
        return {};
    }

    bool inCorrectGroup = false;

    Application app;
    app.lastModified = fileInfo.lastModified().toSecsSinceEpoch();
    app.exec = fileInfo.fileName();
    while (!file.atEnd()) {
        QString line = file.readLine().simplified();

        if (line.startsWith('[')) {
            inCorrectGroup = (line == "[Desktop Entry]");
            continue;
        }

        if (!inCorrectGroup) {
            continue;
        }

        if (line.startsWith("Name") && !line.contains('[')) {
            line.remove(0, line.indexOf('=') + 1);
            app.name = line;
            continue;
        }
        if (line.startsWith("Keywords") && !line.contains('[')) {
            line.remove(0, line.indexOf('=') + 1);
            app.keywords = line;
            continue;
        }


        if (line.startsWith("Icon")) {
            line.remove(0, line.indexOf('=') + 1);
            app.icon = line;
            continue;
        }

        if (line.startsWith("Exec")) {
            line.remove(0, line.indexOf('=') + 1);
            if (line.isEmpty()) {
                continue;
            }

            app.exec = line.trimmed();
            continue;
        }

        if (line.startsWith("NoDisplay=") && line.contains("true", Qt::CaseInsensitive)) {
            return {};
        }
    }
    if (!app.keywords.contains(app.name, Qt::CaseInsensitive)) {
        app.keywords += ';' + app.name;
    }
    if (!app.keywords.contains(app.exec, Qt::CaseInsensitive)) {
        app.keywords += ';' + app.exec;
    }
    app.keywords = app.keywords.toLower();

    if (app.icon.isEmpty()) {
        app.icon = "application-x-executable";
    }

    if (app.name.isEmpty()) {
        qWarning() << "Missing name" << fileInfo.fileName() << app.exec;
        app.name = app.exec;
    }

    return app;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
