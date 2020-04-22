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

#include "Mangonel.h"

#include <QCryptographicHash>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QIcon>
#include <QMenu>
#include <QTextDocument>
#include <QClipboard>
#include <QSettings>
#include <QDateTime>
#include <QQmlEngine>
#include <QElapsedTimer>

#include <KLocalizedString>
#include <KNotification>
#include <KNotifyConfigWidget>
#include <KGlobalAccel>

#include "Config.h"
//Include the providers.
#include "providers/Applications.h"
#include "providers/Paths.h"
#include "providers/Shell.h"
#include "providers/Calculator.h"
#include "providers/Units.h"

#include <QDebug>

#include <unistd.h>

static QHash<QString, Popularity> getRecursivePopularity(QSettings &settings, const QString &path = QString())
{
    QHash<QString, Popularity> ret;
    for (const QString &key : settings.childGroups()) {
        settings.beginGroup(key);
        const QString program = path + key;
        Popularity pop;
        pop.count = settings.value("launches").toLongLong();
        pop.lastUse = settings.value("lastUse").toLongLong();
        pop.matchStrings = settings.value("matchStrings").toStringList();

        if (pop.count || pop.lastUse || !pop.matchStrings.isEmpty()) {
            ret.insert(program, pop);
        }

        for (const QString &child : settings.childGroups()) {
            settings.beginGroup(child);
            const QString childPath = (path.isEmpty() ? "/" : "") + path + key + "/" + child + "/";
            QHash<QString, Popularity> children = getRecursivePopularity(settings, childPath);
            QHash<QString, Popularity>::const_iterator i = children.constBegin();
            while (i != children.constEnd()) {
                ret.insert(i.key(), i.value());
                ++i;
            }
            settings.endGroup();
        }

        settings.endGroup();
    }
    return ret;
}

Mangonel::Mangonel()
{
    // Setup our global shortcut.
    m_actionShow = new QAction(i18n("Show Mangonel"), this);
    m_actionShow->setObjectName(QString("show"));
    QList<QKeySequence> shortcuts({QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Space)});
    KGlobalAccel::self()->setShortcut(m_actionShow, shortcuts);
    shortcuts = KGlobalAccel::self()->shortcut(m_actionShow);
    connect(m_actionShow, SIGNAL(triggered()), this, SIGNAL(triggered()));

    QString shortcutString;
    if (!shortcuts.isEmpty()) {
        shortcutString = shortcuts.first().toString();
    }

    QString message = xi18nc("@info", "Press <shortcut>%1</shortcut> to show Mangonel.", shortcutString);
    KNotification::event(QLatin1String("startup"), message);

    QSettings settings;
    m_history = settings.value("history").toStringList();

    // Instantiate the providers.
    m_providers["applications"] = new Applications(this);
    m_providers["paths"] = new Paths(this);
    m_providers["shell"] = new Shell(this);
    m_providers["Calculator"] = new Calculator(this);
    m_providers["Units"] = new Units(this);

    // Migrate old and broken
    if (settings.childGroups().contains("popularities")) {
        settings.beginGroup("popularities");
        QHash<QString, Popularity> children = getRecursivePopularity(settings);
        QHash<QString, Popularity>::const_iterator i = children.constBegin();
        while (i != children.constEnd()) {
            m_popularities.insert(i.key(), i.value());
            ++i;
        }
        settings.endGroup();
    }

    settings.beginGroup("popularitiesv2");
    for (const QString &key : settings.childGroups()) {
        settings.beginGroup(key);
        Popularity pop;
        const QString program = settings.value("program").toString();
        pop.count = settings.value("launches").toLongLong();
        pop.lastUse = settings.value("lastUse").toLongLong();
        pop.matchStrings = settings.value("matchStrings").toStringList();
        m_popularities.insert(program, pop);
        settings.endGroup();
    }
    settings.endGroup();
}

void Mangonel::storePopularities()
{
    QSettings settings;
    settings.beginGroup("popularitiesv2");
    for (const QString &key : m_popularities.keys()) {
        // QSettings is dumb and annoying
        const QByteArray hashedProgram = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex();
        settings.beginGroup(QString::fromLatin1(hashedProgram));
        settings.setValue("program", key);
        settings.setValue("launches", m_popularities[key].count);
        settings.setValue("lastUse", m_popularities[key].lastUse);
        settings.setValue("matchStrings", m_popularities[key].matchStrings);
        settings.endGroup();
    }
    settings.endGroup();

    // Remove legacy
    settings.beginGroup("popularities");
    settings.remove("");
}

Mangonel *Mangonel::instance()
{
    static Mangonel s_instance;
    return &s_instance;
}

QList<QObject *> Mangonel::setQuery(const QString &query)
{
    m_currentQuery = query;

    if (query.isEmpty()) {
        return {};
    }

    const qint64 currentSecsSinceEpoch = QDateTime::currentSecsSinceEpoch();

    QElapsedTimer timer;

    m_current = -1;
    QList<ProviderResult*> newResults;
    for (Provider* provider : m_providers) {
        timer.restart();
        QList<ProviderResult*> list = provider->getResults(query);
        if (timer.elapsed() > 30) {
            qWarning() << provider << "spent" << timer.elapsed() << "ms on" << query;
        }
        for (ProviderResult *app : list) {
            if (!app) {
                qWarning() << "got null app from" << provider;
                continue;
            }
            if (app->name.isEmpty()) {
                qWarning() << "empty name!" << app->name << app->program << app->completion;
                continue;
            }
            QQmlEngine::setObjectOwnership(app, QQmlEngine::JavaScriptOwnership);
            app->setParent(this);

            if (!app->isCalculation) {
                if (m_popularities.contains(app->program)) {
                    const Popularity &popularity = m_popularities[app->program];
                    app->priority = currentSecsSinceEpoch - popularity.lastUse;
                    app->priority -= (3600 * 360) * popularity.count;
                }
            }

            newResults.append(app);
        }
    }

    std::sort(newResults.begin(), newResults.end(), [this](ProviderResult *a, ProviderResult *b) {
            Q_ASSERT(a);
            Q_ASSERT(b);

            if (a->isCalculation != b->isCalculation) {
                return a->isCalculation;
            }

            const bool aContains = a->name.contains(m_currentQuery, Qt::CaseInsensitive) ||
                                    a->program.contains(m_currentQuery, Qt::CaseInsensitive);
            const bool bContains = b->name.contains(m_currentQuery, Qt::CaseInsensitive) ||
                                    b->program.contains(m_currentQuery, Qt::CaseInsensitive);
            if (aContains != bContains) {
                return aContains;
            }

            const bool aHasPopularity = m_popularities.contains(a->program);
            const bool bHasPopularity = m_popularities.contains(b->program);
            if (aHasPopularity != bHasPopularity) {
                return aHasPopularity;
            }

            if (aHasPopularity && bHasPopularity) {
                const Popularity &aPopularity = m_popularities[a->program];
                const Popularity &bPopularity = m_popularities[b->program];

                const bool aHasMatchStrings = aPopularity.matchStrings.contains(m_currentQuery);
                const bool bHasMatchStrings = bPopularity.matchStrings.contains(m_currentQuery);
                if (aHasMatchStrings != bHasMatchStrings) {
                    return aHasMatchStrings;
                }

                if (aPopularity.count != bPopularity.count) {
                    return aPopularity.count > bPopularity.count;
                }

                if (aPopularity.lastUse != bPopularity.lastUse) {
                    return aPopularity.lastUse > bPopularity.lastUse;
                }
            }

            bool aStartMatch = a->name.startsWith(m_currentQuery, Qt::CaseInsensitive);
            bool bStartMatch = b->name.startsWith(m_currentQuery, Qt::CaseInsensitive);
            if (aStartMatch != bStartMatch) {
                return aStartMatch;
            }

            aStartMatch = a->program.startsWith(m_currentQuery, Qt::CaseInsensitive);
            bStartMatch = b->program.startsWith(m_currentQuery, Qt::CaseInsensitive);
            if (aStartMatch != bStartMatch) {
                return aStartMatch;
            }

            if (a->priority != b->priority) {
                return a->priority < b->priority;
            }

            if (a->name != b->name) {
                return a->name > b->name;
            }

            // They are 100% equal
            return false;
    });

    QList<QObject*> ret;
    for (ProviderResult *result : newResults) {
        ret.append(result);
    }

    return ret;
}

void Mangonel::launch(QObject *selectedObject)
{
    ProviderResult *selected = qobject_cast<ProviderResult*>(selectedObject);
    if (!selected) {
        qWarning() << "Trying to launch null pointer";
        return;
    }

    addToHistory(m_currentQuery);
    selected->launch();

    if (selected->isCalculation) {
        return;
    }

    Popularity pop;
    const QString exec = selected->program;

    if (m_popularities.contains(exec)) {
        pop = m_popularities[exec];
        pop.lastUse = QDateTime::currentSecsSinceEpoch();

        // Cap it, so history doesn't haunt forever
        pop.count = std::min(pop.count + 1, qint64(10));

        if (pop.matchStrings.contains(m_currentQuery)) {
            pop.matchStrings.removeAll(m_currentQuery);
        }
    } else {
        pop.lastUse = QDateTime::currentSecsSinceEpoch();
        pop.count = 0;
    }
    pop.matchStrings.prepend(m_currentQuery);

    m_popularities[exec] = pop;

    storePopularities();
}

void Mangonel::showConfig()
{
    QList<QKeySequence> shortcuts(KGlobalAccel::self()->globalShortcut(qApp->applicationName(), "show"));
    ConfigDialog* dialog = new ConfigDialog;
    if (!shortcuts.isEmpty()) {
        dialog->setHotkey(shortcuts.first());
    }
    connect(dialog, SIGNAL(hotkeyChanged(QKeySequence)), this, SLOT(setHotkey(QKeySequence)));
    dialog->exec();
}

void Mangonel::setHotkey(const QKeySequence& hotkey)
{
    KGlobalAccel::self()->setShortcut(m_actionShow,
                                      QList<QKeySequence>() << hotkey,
                                      KGlobalAccel::NoAutoloading);
}

void Mangonel::configureNotifications()
{
    KNotifyConfigWidget::configure();
}

QString Mangonel::selectionClipboardContent()
{
    return QGuiApplication::clipboard()->text(QClipboard::Selection);
}

void Mangonel::addToHistory(const QString &text)
{
    m_history.prepend(text);
    emit historyChanged();

    // Store history of session.
    QSettings settings;
    settings.setValue("history", m_history);
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
