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
        qWarning() << "Failed to open" << fileInfo.fileName();
        return {};
    }

    bool inCorrectGroup = false;

    Application app;
    app.lastModified = fileInfo.lastModified().toSecsSinceEpoch();

    // Don't necessarily need to change this all the time, but the language can
    // in theory be changed by the user. And this function isn't called often
    // enough to be in a hot path.
    QSet<QString> wantedLanguages;
    { // Because Qt suddenly decided that being True C++™ was more important than ease of use
        const QStringList languages = QLocale::system().uiLanguages();
        wantedLanguages = QSet<QString>(languages.begin(), languages.end());
        wantedLanguages.insert(QLocale::system().bcp47Name());
        wantedLanguages.insert("en");
    }

    while (!file.atEnd()) {
        const QString line = QString::fromLocal8Bit(file.readLine()).simplified();

        if (line.startsWith('[')) {
            inCorrectGroup = (line == "[Desktop Entry]");
            continue;
        }

        if (!inCorrectGroup) {
            continue;
        }

        const int separatorPos = line.indexOf('=');
        if (separatorPos == -1) {
            continue;
        }
        const QStringRef value = line.midRef(separatorPos + 1);

        const int openBracketPos = line.indexOf('[');

        // There is a bracket in the Key part, e. g. Foo[lang]=asdf
        // These are translated versions of the entries.
        if (openBracketPos != -1 && openBracketPos < separatorPos) {
            const int closeBracketPos = line.indexOf(']');
            if (closeBracketPos < openBracketPos) {
                qWarning() << "Invalid brackets in" << fileInfo.fileName() << line;
                continue;
            }
            if (!wantedLanguages.contains(line.mid(openBracketPos + 1, closeBracketPos - openBracketPos - 1))) {
                continue;
            }
        }

        if (line.startsWith("Name")) {
            app.name = value.toString();
            continue;
        }
        if (line.startsWith("Keywords")) {
            app.keywords = value.toString();
            continue;
        }


        if (line.startsWith("Icon")) {
            app.icon = value.toString();
            continue;
        }

        if (line.startsWith("Exec")) {
            app.exec = value.toString();
            continue;
        }

        if (line.startsWith("NoDisplay=") && line.contains("true", Qt::CaseInsensitive)) {
            return {};
        }
    }

    if (app.name.isEmpty()) {
        qWarning() << fileInfo.absoluteFilePath() << "missing name";
        app.name = fileInfo.baseName();
    }

    if (!app.isValid()) {
        return {};
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

    return app;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
