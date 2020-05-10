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
#include <QDebug>

#include <sys/stat.h>
#include <dirent.h>

Paths::Paths(QObject *parent) :
    Provider(parent)
{
}

Paths::~Paths()
{}

// QDir::entryList is very slow, so we do it ourself (on Linux, someone who knows the others need to fix them)
static QStringList sortedDirList(const QString &dirPath, const QString &matchPrefix)
{
#ifdef Q_OS_LINUX
    QByteArray path = dirPath.toLocal8Bit();
    if (!path.endsWith('/')) {
        path += '/';
    }
    DIR *dir = opendir(path.constData());
    if (!dir) {
        qWarning() << "Error opening dir:" << strerror(errno);
        return {};
    }
    QHash<QString, qint64> entries;
    struct stat statbuf;
    dirent *ent;
    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.') {
            continue;
        }
        const QString name = QString::fromLocal8Bit(ent->d_name);
        if (!name.startsWith(matchPrefix, Qt::CaseInsensitive)) {
            continue;
        }
        QByteArray new_path = path + static_cast<const char*>(ent->d_name);
        if (lstat(new_path.constData(), &statbuf) == -1) {
            qWarning() << "Error stating file:" << strerror(errno);
            continue;
        }
        if (S_ISCHR(statbuf.st_mode) ||
                S_ISBLK(statbuf.st_mode) ||
                S_ISFIFO(statbuf.st_mode)||
                S_ISSOCK(statbuf.st_mode)) {
            continue;
        }
        entries[name] = statbuf.st_mtime;
    }
    closedir(dir);
    QStringList names = entries.keys();
    std::sort(names.begin(), names.end(), [&](const QString &a, const QString &b) {
            return entries[a] > entries[b];
            });
    return names;
#else
    return QDir(dirPath).entryList(QStringList(matchPrefix + '*'), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::Time);
#endif
}

QList<ProviderResult *> Paths::getResults(QString query)
{
    QList<ProviderResult*> list;

    QDir dir;
    if (query.startsWith("/")) {
        dir = QDir::root();
        query.remove(0, 1);
    } else {
        dir = QDir::home();

        if (query.startsWith('~')) {
            query.remove(0, 1);
        }
    }

    QStringList walk = query.split("/", QString::SkipEmptyParts);
    if (walk.isEmpty() || query.endsWith('/')) {
        walk.append("");
    }

    QString part = walk.takeFirst();
    while (walk.length() > 0) {
        if (!dir.exists(part)) {
            return list;
        }

        dir.cd(part);
        part = walk.takeFirst();
    }

    QStringList paths = sortedDirList(dir.path(), part);

    if (paths.size() > 100) {
        paths = paths.mid(0, 100);
    }

    qint64 currentSecsSinceEpoch = QDateTime::currentSecsSinceEpoch();

    static const QString homePath = QDir::homePath();

    for(const QString &name : paths) {
        const QFileInfo path(dir.filePath(name));
        ProviderResult *result = new ProviderResult();

        result->program = path.absoluteFilePath();

        if (result->program.startsWith(homePath)) {
            result->name = result->program.mid(homePath.length());
            result->completion = "~" + result->name.left(result->name.lastIndexOf("/")) + "/" + path.fileName();
        } else {
            result->name = result->program;
            result->completion = result->name.left(result->name.lastIndexOf("/")) + "/" + path.fileName();
        }
        result->priority = currentSecsSinceEpoch - path.lastModified().toSecsSinceEpoch();
        if (path.isDir()) {
            result->completion += "/";
            result->icon = "system-file-manager";
        } else {
            result->icon = m_mimeDb.mimeTypeForFile(path).iconName();
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
