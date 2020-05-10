/*
 * Copyright 2010-2012 Bart Kroon <bart@tarmack.eu>
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

#include "Shell.h"

#include <stdlib.h>
#include <QDBusInterface>
#include <QDir>
#include <klocalizedstring.h>
#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QFileSystemWatcher>

void Shell::walkDir(QString path)
{
    QDir dir = QDir(QFileInfo(path).canonicalFilePath());
    QFileInfoList list = dir.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::Files);

    for (const QFileInfo &file : list) {
        const QString canonicalPath = file.canonicalFilePath();

        if (file.isFile() && file.isExecutable()) {
            m_index[file.fileName()] = canonicalPath;
            m_modified[file.fileName()] = file.lastModified().toSecsSinceEpoch();
        }
    }
}

namespace
{
QStringList parsePathEnv()
{
    QString pathEnv = getenv("PATH");
    QStringList pathList = pathEnv.split(":", QString::SkipEmptyParts);
    QStringList absPathList;
    for (const QString &path : pathList) {
        const QString canonicalPath = QFileInfo(path).canonicalFilePath();
        if (canonicalPath.isEmpty()) {
            continue;
        }
        absPathList.append(canonicalPath);
    }

    absPathList.removeDuplicates();
    return absPathList;
}
}

Shell::Shell(QObject *parent) :
    Provider(parent),
    m_fsWatcher(new QFileSystemWatcher(this))
{
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &Shell::onDirectoryChanged);

    for (const QString &dir : parsePathEnv()) {
        m_fsWatcher->addPath(dir);
        walkDir(dir);
    }
}

Shell::~Shell()
{}

QList<ProviderResult *> Shell::getResults(QString query)
{
    QList<ProviderResult*> list;

    QString command;
    QString args;

    if (query.contains(' ')) {
        command = query.left(query.indexOf(" "));
        args = query.right(query.length() - query.indexOf(" "));
    } else {
        command = query;
    }

    const qint64 currentSecsSinceEpoch = QDateTime::currentSecsSinceEpoch();
    QHashIterator<QString, QString> iterator(this->m_index);
    while (iterator.hasNext()) {
        iterator.next();

        if (!iterator.key().startsWith(command, Qt::CaseInsensitive)) {
            continue;
        }

        ProviderResult *app = new ProviderResult;
        app->name = iterator.value() + args;
        app->completion = iterator.key();
        app->icon = "system-run";
        app->object = this;
        app->program = iterator.value() + args;
        app->type = i18n("Shell command");

        app->priority = currentSecsSinceEpoch - m_modified[iterator.key()];

        list.append(app);
    }
    return list;
}

int Shell::launch(const QString &exec)
{
    QStringList args = exec.split(" ");
    if (args.isEmpty()) {
        qWarning() << "Asked to launch invalid program:" << exec;
        return 0;
    }
    QString program(args.takeFirst());
    QProcess::startDetached(program, args);
    return 0;
}

void Shell::onDirectoryChanged(const QString &path)
{
    // First remove the old
    QStringList toRemove;
    for (const QString &fullProgramPath : m_index.values()) {
        if (!fullProgramPath.startsWith(path)) {
            continue;
        }

        QFileInfo programInfo(fullProgramPath);
        if (programInfo.dir().path() == path) {
            toRemove.append(programInfo.fileName());
        }
    }

    // Too lazy to use an iterator
    for (const QString &program : toRemove) {
        m_index.remove(program);
        m_modified.remove(program);
    }

    // Then re-index the path
    walkDir(path);
}


// kate: indent-mode cstyle; space-indent on; indent-width 4; 
