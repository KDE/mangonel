/*
 * Copyright 2010, 2011 Bart Kroon <bart@tarmack.eu>
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

#include "Config.h"

#include <QDialogButtonBox>
#include <QKeySequenceEdit>
#include <KAutostart>
#include <QSettings>
#include <QLabel>
#include <klocalizedstring.h>
#include <QCheckBox>
#include <QGridLayout>

ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent, Qt::Dialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Mangonel Configuration"));

    delete layout();
    QGridLayout *layout = new QGridLayout;
    setLayout(layout);
    layout->setAlignment(Qt::AlignHCenter);

    // Shortcut
    // TODO: use normal global shortcut stuff
    QLabel* hotkeyLabel = new QLabel(i18n("Shortcut to show Mangonel:"), this);
    layout->addWidget(hotkeyLabel, 0, 0);
    m_hotkeySelect = new QKeySequenceEdit(this);
    this->connect(m_hotkeySelect, SIGNAL(keySequenceChanged(const QKeySequence&)), SIGNAL(hotkeyChanged(const QKeySequence&)));
    layout->addWidget(m_hotkeySelect, 0, 1);

    QLabel* autostartLabel = new QLabel(i18n("Automatically launch Mangonel on login:"), this);
    layout->addWidget(autostartLabel, 1, 0);
    QCheckBox *autostartCheck = new QCheckBox(this);
    KAutostart as;
    autostartCheck->setChecked(as.autostarts());
    this->connect(autostartCheck, SIGNAL(toggled(bool)), SLOT(setAutostart(bool)));
    layout->addWidget(autostartCheck, 1, 1);

    // Add close button.
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    this->connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttons);
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setHotkey(QKeySequence hotkey)
{
    m_hotkeySelect->setKeySequence(hotkey);
}

void ConfigDialog::setAutostart(bool autostart)
{
    KAutostart as;
    as.setAutostarts(autostart);
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
