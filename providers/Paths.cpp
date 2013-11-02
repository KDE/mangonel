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

#include <QDir>
#include <KDE/KFileItem>
#include <klocale.h>


Paths::Paths()
{}

Paths::~Paths()
{}

QList<Application> Paths::getResults(QString query)
{
    QList<Application> list = QList<Application>();
    QString original = query;
    QDir dir;
    if (query.startsWith("/"))
    {
        dir = QDir::root();
        query.remove(0, 1);
    }
    else
        dir = QDir::home();
    QStringList walk = query.split("/", QString::SkipEmptyParts);
    if (walk.isEmpty())
        walk.append("");
    QString part = walk.takeFirst();
    while (walk.length() > 0)
    {
        dir.cd(part);
        part = walk.takeFirst();
    }
    query = part + "*";
    QFileInfoList paths = dir.entryInfoList(QStringList(query), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach(QFileInfo path, paths)
    {
        Application result = Application();
        result.name = subUser(path.absoluteFilePath());
        result.completion = result.name.left(result.name.lastIndexOf("/")) + "/" + path.fileName();
        KFileItem info = KFileItem(KFileItem::Unknown, KFileItem::Unknown, path.absoluteFilePath());
        if (path.isDir())
        {
            result.completion += "/";
            result.icon = "system-file-manager";
            result.priority = time(NULL) - info.time(KIO::UDSEntry::UDS_MODIFICATION_TIME);
        }
        else
        {
            result.icon = info.iconName();
            result.priority = time(NULL) - info.time(KIO::UDSEntry::UDS_MODIFICATION_TIME);
        }
        result.object = this;
        result.program = path.absoluteFilePath();
        result.type = i18n("Open path");
        list.append(result);
    }
    return list;
}

int Paths::launch(QVariant selected)
{
    KFileItem info = KFileItem(KFileItem::Unknown, KFileItem::Unknown, selected.toString());
    info.run();
    return 0;
}


namespace
{
QString subUser(QString path)
{
    QString homePath = QDir::homePath();
    if (path.startsWith(homePath))
        path = "~" + path.mid(homePath.length(), -1);
    return path;
}
};


#include "Paths.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
