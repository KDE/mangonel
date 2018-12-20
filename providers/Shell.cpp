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

namespace
{
QMap<QString, QString> walkDir(QString path)
{
    QMap<QString, QString> binList;
    QDir dir = QDir(path);
    QFileInfoList list = dir.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    for (const QFileInfo &file : list) {
        if (file.isDir()) {
            if (file.isSymLink() && file.canonicalFilePath() != path) {
                binList.unite(walkDir(file.absoluteFilePath()));
            }
        } else {
            if (file.isExecutable()) {
                binList.insert(file.fileName(), file.absoluteFilePath());
            }
        }
    }

    return binList;
}

QStringList getPathEnv()
{
    QString pathEnv = getenv("PATH");
    QStringList pathList = pathEnv.split(":", QString::SkipEmptyParts);
    pathList.append(QDir::homePath() + "/bin");
    pathList.removeDuplicates();
    return pathList;
}
}

Shell::Shell(QObject *parent) :
    Provider(parent)
{
    index.clear();

    for (const QString &dir : getPathEnv()) {
        index.unite(walkDir(dir));
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

    QMapIterator<QString, QString> iterator(this->index);
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

        app->priority = QDateTime::currentSecsSinceEpoch() - QFileInfo(iterator.value()).lastModified().toSecsSinceEpoch();

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


// kate: indent-mode cstyle; space-indent on; indent-width 4; 
