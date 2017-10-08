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

Units::Units(QObject *parent) :
    Provider(parent)
{
}

Units::~Units()
{}

QList<Application *> Units::getResults(QString query)
{
    QList<Application*> list;

    QRegExp pattern("(\\d+)\\s*(\\w+)\\s+(?:\\=|to|is|in)\\s+(\\w+)$", Qt::CaseInsensitive);
    if (!query.contains(pattern) || pattern.captureCount() != 3) {
        return list;
    }

    const KUnitConversion::Unit inputUnit = resolveUnitName(pattern.cap(2));
    if (!inputUnit.isValid()) {
        return list;
    }

    const double inputNumber = pattern.cap(1).toDouble();

    const KUnitConversion::Value inputValue(inputNumber, inputUnit);
    const KUnitConversion::Unit outputUnit = resolveUnitName(pattern.cap(3), inputUnit.category());
    if (!outputUnit.isValid()) {
        return list;
    }

    const KUnitConversion::Value outputValue = m_converter.convert(inputValue, outputUnit);

    Application *result = new Application;
    result->icon = "accessories-calculator";
    result->object = this;
    result->name = i18nc("conversion from one unit to another", "%1 is %2").arg(inputValue.toString()).arg(outputValue.toString());
    result->program = result->name;
    result->type = i18n("Unit conversion");
    list.append(result);

    return list;
}

int Units::launch(QVariant selected)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(selected.toString(), QClipboard::Selection);
    return 0;
}

KUnitConversion::Unit Units::resolveUnitName(const QString &name, const KUnitConversion::UnitCategory &category)
{
    KUnitConversion::Unit unit;

    if (!category.isNull()) {
        unit = category.unit(name);
    } else {
        unit = m_converter.unit(name);
    }

    if (unit.isValid()) {
        return unit;
    }

    // Didn't match directly, try to match without case sensitivity

    if (!category.isNull()) {
        // try only common first
        unit = matchUnitCaseInsensitive(name, category, OnlyCommonUnits);
        if (unit.isValid()) {
            return unit;
        }

        // If not common, try something else
        unit = matchUnitCaseInsensitive(name, category, AllUnits);
        if (unit.isValid()) {
            return unit;
        }
    } else {
        for (const KUnitConversion::UnitCategory &candidateCategory : m_converter.categories()) {
            unit = matchUnitCaseInsensitive(name, candidateCategory, OnlyCommonUnits);
            if (unit.isValid()) {
                return unit;
            }
        }

        // Was not a common unit, try all units
        for (const KUnitConversion::UnitCategory &candidateCategory : m_converter.categories()) {
            unit = matchUnitCaseInsensitive(name, candidateCategory, AllUnits);
            if (unit.isValid()) {
                return unit;
            }
        }
    }

    return unit;
}

KUnitConversion::Unit Units::matchUnitCaseInsensitive(const QString &name, const KUnitConversion::UnitCategory &category, const UnitMatchingLevel level)
{
    if (category.isNull()) {
        return KUnitConversion::Unit();
    }

    QSet<KUnitConversion::UnitId> commonIds;
    if (level == OnlyCommonUnits) {
        for (const KUnitConversion::Unit &unit : category.mostCommonUnits()) {
            commonIds.insert(unit.id());
        }
    }

    for (const QString &candidateName : category.allUnits()) {
        if (name.compare(candidateName, Qt::CaseInsensitive)) {
            continue;
        }

        KUnitConversion::Unit candidate = category.unit(candidateName);
        if (level == OnlyCommonUnits && !commonIds.contains(candidate.id())) {
            continue;
        }

        if (candidate.isValid()) {
            return candidate;
        }
    }

    return KUnitConversion::Unit();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
