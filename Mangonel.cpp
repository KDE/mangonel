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

#include <QGuiApplication>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QIcon>
#include <QMenu>
#include <QTextDocument>
#include <QClipboard>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>
#include <KNotifyConfigWidget>
#include <KGlobalAccel>
#include <KSharedConfig>

#include "Config.h"
//Include the providers.
#include "providers/Applications.h"
#include "providers/Paths.h"
#include "providers/Shell.h"
#include "providers/Calculator.h"
#include "providers/Units.h"

#include <QDebug>

#include <unistd.h>

#define WINDOW_WIDTH 440
#define WINDOW_HEIGHT 400
#define ICON_SIZE (WINDOW_WIDTH / 1.5)

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

    QString message = i18nc("@info", "Press <shortcut>%1</shortcut> to show Mangonel.", shortcutString);
    KNotification::event(QLatin1String("startup"), message);

    const KConfigGroup config(KSharedConfig::openConfig(), "mangonel_main");
    m_history = config.readEntry("history", QStringList());

    // Instantiate the providers.
    m_providers["applications"] = new Applications(this);
    m_providers["paths"] = new Paths(this);
    m_providers["shell"] = new Shell(this);
    m_providers["Calculator"] = new Calculator(this);
    m_providers["Units"] = new Units(this);
}

Mangonel::~Mangonel()
    // Store history of session.
{
    KConfigGroup config(KSharedConfig::openConfig(), "mangonel_main");
    config.writeEntry("history", m_history);
    config.config()->sync();
}

Mangonel *Mangonel::instance()
{
    static Mangonel s_instance;
    return &s_instance;
}

QList<QObject *> Mangonel::apps()
{
    QList<QObject*> ret;

    for (QPointer<Application> app : m_apps) {
        ret.append(app.data());
    }

    return ret;
}

void Mangonel::getApp(QString query)
{
    for (QPointer<Application> app : m_apps) {
        if (!app) {
            continue;
        }

        app->deleteLater();
    }
    m_apps.clear();

    if (query.length() > 0) {
        m_current = -1;
        for (Provider* provider : m_providers) {
            QList<Application*> list = provider->getResults(query);
            for (Application *app : list) {
                app->setParent(this);
                m_apps.append(app);
            }
        }

        std::sort(m_apps.begin(), m_apps.end(), [](Application *a, Application *b) {
            if (a->priority != b->priority) {
                return a->priority < b->priority;
            } else {
                return a->name > b->name;
            }
        });
    }
    emit appsChanged();
}

void Mangonel::showConfig()
{
    QList<QKeySequence> shortcuts(KGlobalAccel::self()->globalShortcut(qApp->applicationName(), "show"));
    ConfigDialog* dialog = new ConfigDialog;
    if (!shortcuts.isEmpty()) {
        dialog->setHotkey(shortcuts.first());
    }
    connect(dialog, SIGNAL(hotkeyChanged(QKeySequence)), this, SLOT(setHotkey(QKeySequence)));
    installEventFilter(this);

    dialog->exec();
    removeEventFilter(this);
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
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
