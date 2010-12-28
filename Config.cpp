#include "Config.h"

#include <qdialogbuttonbox.h>
#include <kkeysequencewidget.h>
#include <QDebug>
#include <QSettings>
#include <QLabel>


ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent, Qt::Dialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_QuitOnClose, false);
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);

    // Make hotkey layout.
    QHBoxLayout* hotkey = new QHBoxLayout(this);
    hotkey->setAlignment(Qt::AlignHCenter);
    hotkey->setSpacing(10);
    // Add text label.
    QLabel* hotkeyLabel = new QLabel("Shortcut to show Mangonel:", this);
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