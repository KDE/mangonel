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

namespace
{
QMap<QString, QString> walkDir(QString path)
{
    QMap<QString, QString> binList;
    QDir dir = QDir(path);
    QFileInfoList list = dir.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach(QFileInfo file, list)
    {
        if (file.isDir())
        {
            if (file.isSymLink() && file.canonicalFilePath() != path)
                binList.unite(walkDir(file.absoluteFilePath()));
        }
        else
        {
            if (file.isExecutable())
                binList.insert(file.fileName(), file.absoluteFilePath());
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
    this->index.clear();
    foreach(QString dir, getPathEnv())
    {
        this->index.unite(walkDir(dir));
    }
}

Shell::~Shell()
{}

QList<Application *> Shell::getResults(QString query)
{
    QList<Application*> list;
    int priority = 3600;
    foreach(QString key, this->index.keys()) {
        if (key.startsWith(query.left(query.indexOf(" ")), Qt::CaseInsensitive)) {
            QString args = query.right(query.length() - query.indexOf(" "));
            if (args == query)
                args = "";
            Application *app = new Application;
            app->name = key + args;
            app->completion = key;
            app->icon = "system-run";
            app->object = this;
            app->program = this->index[key] + args;
            app->type = i18n("Shell command");
            app->priority = ++priority;

            list.append(app);
        }
    }
    return list;
}

int Shell::launch(QVariant selected)
{
    QStringList args = selected.toString().split(" ");
    if (args.isEmpty()) {
        qWarning() << "Asked to launch invalid program:" << selected;
        return 0;
    }
    QString program(args.takeFirst());
    QProcess::startDetached(program, args);
    return 0;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4; 
