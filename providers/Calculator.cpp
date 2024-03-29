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

#include "Calculator.h"

#include <assert.h>
#include <math.h>
#include <QApplication>
#include <QClipboard>
#include <QStringList>
#include <klocalizedstring.h>
#include <QDebug>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "calculator/evaluator.h"
#include "calculator/numberformatter.h"
#include "calculator/settings.h"

#include <iostream>

Calculator::Calculator(QObject *parent) :
    Provider(parent)
{
    Settings::instance()->resultPrecision = 5;
    Evaluator::instance();
}

Calculator::~Calculator()
{}

QList<ProviderResult*> Calculator::getResults(QString query)
{
    // Replace #x# with #*#, where # are numbers, consider e. g. 32x32 the same as 32*32
    QRegularExpressionMatchIterator xMatches = QRegularExpression(R"([0-9]+\s*(x)\s*[0-9]+)").globalMatch(query);
    while (xMatches.hasNext()) {
        const QRegularExpressionMatch match = xMatches.next();
        query.replace(match.capturedStart(1), 1, '*');
    }

    Evaluator *ev = Evaluator::instance();
    query = ev->autoFix(query);
    if (query.isEmpty()) {
        return {};
    }

    ev->setExpression(query);

    const Quantity quantity = ev->evalNoAssign();

    if (!ev->error().isEmpty()) {
        return {};
    }

    ProviderResult *app = new ProviderResult;
    if (ev->error().isEmpty()) {
        app->name = NumberFormatter::format(quantity);

        if (app->name.compare(query.trimmed(), Qt::CaseInsensitive) == 0) {
            return {};
        }
        app->program = app->name;
    } else {
        app->name = query + ":\n" + ev->error();
    }

    app->completion = app->name;
    app->icon = "accessories-calculator";
    app->object = this;
    app->type = i18n("Calculation");
    app->isCalculation = true;

    return {app};
}

int Calculator::launch(const QString &exec)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(exec);
    return 0;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
