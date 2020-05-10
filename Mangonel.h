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

#ifndef Mangonel_H
#define Mangonel_H

#include "Provider.h"

#include <QAction>
#include <QObject>
#include <QPointer>

class Mangonel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged)

public:
    static Mangonel *instance();

    const QStringList &history() { return m_history; }

public slots:
    QList<QObject*> setQuery(const QString &query);
    void launch(QObject *selectedObject);
    void showConfig();
    void configureNotifications();
    QString selectionClipboardContent();
    void addToHistory(const QString &text);

signals:
    void appsChanged();
    void triggered();
    void historyChanged();

private slots:
    void setHotkey(const QKeySequence& hotkey);
private:
    Mangonel();
    void storePopularities();

    QAction* m_actionShow;
    QStringList m_history;
    QHash<QString, Provider*> m_providers;

    QHash<QString, Popularity> m_popularities;

    int m_current = -1;
    QString m_currentQuery;
};

#endif // Mangonel_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
