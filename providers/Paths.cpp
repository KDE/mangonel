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

#include "Paths.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

Paths::Paths(QObject *parent) :
    Provider(parent)
{
}

Paths::~Paths()
{}

QList<ProviderResult *> Paths::getResults(QString query)
{
    QList<ProviderResult*> list;

    QDir dir;
    if (query.startsWith("/")) {
        dir = QDir::root();
        query.remove(0, 1);
    } else {
        dir = QDir::home();
    }

    QStringList walk = query.split("/", QString::SkipEmptyParts);
    if (walk.isEmpty() || query.endsWith('/')) {
        walk.append("");
    }

    QString part = walk.takeFirst();
    while (walk.length() > 0) {
        dir.cd(part);
        part = walk.takeFirst();
    }

    QFileInfoList paths = dir.entryInfoList(QStringList(part + '*'), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::Time);

    if (paths.size() > 100) {
        paths = paths.mid(0, 100);
    }

    for(const QFileInfo &path : paths) {
        ProviderResult *result = new ProviderResult();

        result->program = path.absoluteFilePath();

        static const QString homePath = QDir::homePath();
        if (result->program.startsWith(homePath)) {
            result->name = path.absoluteFilePath().mid(homePath.length());
            result->completion = "~" + result->name.left(result->name.lastIndexOf("/")) + "/" + path.fileName();
        } else {
            result->name = path.absoluteFilePath();
            result->completion = result->name.left(result->name.lastIndexOf("/")) + "/" + path.fileName();
        }
        result->priority = QDateTime::currentSecsSinceEpoch() - path.lastModified().toSecsSinceEpoch();
        if (path.isDir()) {
            result->completion += "/";
            result->icon = "system-file-manager";
        } else {
            result->icon = m_mimeDb.mimeTypeForFile(path.absoluteFilePath()).iconName();
        }

        result->object = this;
        result->type = i18n("Open path");
        list.append(result);
    }

    return list;
}

int Paths::launch(const QString &exec)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(exec));
    return 0;
}



// kate: indent-mode cstyle; space-indent on; indent-width 4; 
