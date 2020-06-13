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

#ifndef APPLICATIONS_H
#define APPLICATIONS_H

#include "Provider.h"
#include <QHash>

class QFileSystemWatcher;
class QFileInfo;

class Applications : public Provider
{
    Q_OBJECT
public:
    Applications(QObject *parent);

    ~Applications();

public slots:
    QList<ProviderResult*> getResults(QString query) override;
    int launch(const QString &selected) override;

private slots:
    void onDirectoryChanged(const QString &path);

private:
    struct Application {
        QString name;
        QString exec;
        qint64 lastModified = -1;
        QString icon;
        QString keywords;

        bool isValid() const {
            return !name.isEmpty() &&
                !exec.isEmpty() &&
                lastModified != -1;
        }
    };

    void loadDir(const QString &path);

    ProviderResult *createApp(const Application &service);
    Application loadDesktopFile(const QFileInfo &file);

    QHash<QString, Application> m_applications;
    QFileSystemWatcher *m_fsWatcher;
};

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
