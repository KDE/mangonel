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
#include <KDE/KKeySequenceWidget>
#include <KDE/KApplication>
#include <QSettings>
#include <QLabel>
#include <klocalizedstring.h>


ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent, Qt::Dialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(i18n("Mangonel Configuration"));
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);

    // Make hotkey layout.
    QHBoxLayout* hotkey = new QHBoxLayout(this);
    hotkey->setAlignment(Qt::AlignHCenter);
    hotkey->setSpacing(10);
    // Add text label.
    QLabel* hotkeyLabel = new QLabel(i18n("Shortcut to show Mangonel:"), this);
    hotkey->addWidget(hotkeyLabel);
    // Add key selector.
    hotkeySelect = new KKeySequenceWidget(this);
    this->connect(hotkeySelect, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SIGNAL(hotkeyChanged(const QKeySequence&)));
    hotkey->addWidget(hotkeySelect);
    // Add hotkey layout.
    layout->addLayout(hotkey);

    // Add close button.
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    this->connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttons);

    this->setLayout(layout);
}

ConfigDialog::~ConfigDialog()
{
    delete layout;
}

void ConfigDialog::setHotkey(QKeySequence hotkey)
{
    hotkeySelect->setKeySequence(hotkey);
}


#include "Config.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
