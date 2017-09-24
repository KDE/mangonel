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

#include "Units.h"

#include <QRegExp>
#include <QClipboard>
#include <QApplication>
#include <klocalizedstring.h>
#include <KUnitConversion/Converter>
#include <QDebug>

Units::Units()
{}

Units::~Units()
{}

QList< Application > Units::getResults(QString query)
{
    QList<Application> list = QList<Application>();
    QRegExp pattern = QRegExp("(\\d+)\\s*(\\w+)\\s+(?:\\=|to|is|in)\\s+(\\w+)$", Qt::CaseInsensitive);
    if (query.contains(pattern) && pattern.captureCount() == 3)
    {
        KUnitConversion::Converter converter;
        KUnitConversion::Value value(pattern.cap(1).toDouble(), pattern.cap(2));
        value = converter.convert(value, pattern.cap(3));
        
        if (value.isValid()) {
            Application result;
            result.icon = "accessories-calculator";
            result.object = this;
            result.name = value.toString();
            result.program = result.name;
            result.type = i18n("Unit conversion");
            list.append(result);
        }
    }
    return list;
}

int Units::launch(QVariant selected)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(selected.toString(), QClipboard::Selection);
    return 0;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
