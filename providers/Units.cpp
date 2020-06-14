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

#include <QRegularExpression>
#include <QClipboard>
#include <QApplication>
#include <klocalizedstring.h>
#include <KUnitConversion/Converter>
#include <cmath>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>

#include "calculator/evaluator.h"

void CurrencyRefresher::init()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(86390 * 1000);
    refresh();
    connect(thread(), &QThread::finished, m_timer, &QTimer::stop);
    m_timer->start();
}

void CurrencyRefresher::refresh()
{
    QElapsedTimer timer; timer.start();
    qDebug() << "Warming up currency converter";
    KUnitConversion::Converter converter;
    KUnitConversion::UnitCategory currency = converter.category(KUnitConversion::CurrencyCategory);
    const KUnitConversion::Unit nok = currency.unit(KUnitConversion::UnitId::Nok);
    const KUnitConversion::Value value = KUnitConversion::Value(1, nok);
    converter.convert(value, KUnitConversion::UnitId::Usd);
    if (timer.elapsed() > 0) {
        qDebug() << "Currency converter warmed in" << timer.elapsed() << "ms";
    }
}

Units::Units(QObject *parent) :
    Provider(parent)
{
    m_refresher = new CurrencyRefresher;
    m_refresherThread = new QThread(this);
    m_refresher->moveToThread(m_refresherThread);
    m_refresherThread->start();
    QMetaObject::invokeMethod(m_refresher, &CurrencyRefresher::init);
}

Units::~Units()
{
    m_refresherThread->quit();
    m_refresherThread->wait();
}

QList<ProviderResult *> Units::getResults(QString query)
{
    QList<ProviderResult*> list;

    QRegularExpression pattern(R"raw((.+?)(\w+)\s+(?:\=|to|is|in)\s+(\w+)$)raw", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = pattern.match(query);
    if (!match.hasMatch()) {
        return list;
    }

    const KUnitConversion::Unit inputUnit = resolveUnitName(match.captured(2));
    if (!inputUnit.isValid()) {
        return list;
    }

    QString sourceAmount = match.captured(1);
    bool ok = false;
    double inputNumber = sourceAmount.toDouble(&ok);
    if (!ok) {
        Evaluator *ev = Evaluator::instance();
        sourceAmount = ev->autoFix(sourceAmount);
        ev->setExpression(sourceAmount);
        const Quantity quantity = ev->evalNoAssign();
        if (!ev->error().isEmpty()) {
            return list;
        }
        inputNumber = Rational(quantity.numericValue().real).toDouble();
    }

    const KUnitConversion::Value inputValue(inputNumber, inputUnit);
    const KUnitConversion::Unit outputUnit = resolveUnitName(match.captured(3), inputUnit.category());
    if (!outputUnit.isValid()) {
        return list;
    }

    const KUnitConversion::Value outputValue = m_converter.convert(inputValue, outputUnit);

    int inputPrecision = 1;
    if (inputValue.number() < 1 || inputUnit.categoryId() != KUnitConversion::CurrencyCategory) {
        qDebug() << inputUnit.categoryId();
        qreal calculationResult = inputValue.number();

        if (calculationResult < 100) {
            inputPrecision = 5;
        }

        double scaledResult = calculationResult * std::pow(10, inputPrecision);
        while (inputPrecision > 0 && std::floor(scaledResult) == std::ceil(scaledResult)) {
            inputPrecision--;
            scaledResult = calculationResult * std::pow(10, inputPrecision);
        }
    }
    int outputPrecision = 1;
    if (outputValue.number() < 1 && outputUnit.categoryId() != KUnitConversion::CurrencyCategory) {
        qDebug() << outputUnit.categoryId();
        qreal calculationResult = outputValue.number();

        if (calculationResult < 100) {
            outputPrecision = 5;
        }

        double scaledResult = calculationResult * std::pow(10, outputPrecision);
        while (outputPrecision > 0 && std::floor(scaledResult) == std::ceil(scaledResult)) {
            outputPrecision--;
            scaledResult = calculationResult * std::pow(10, outputPrecision);
        }
    }

    QString inputString = QLocale::system().toString(inputValue.number(), 'f', inputPrecision + 1);
    const QString outputString = QLocale::system().toString(outputValue.number(), 'f', outputPrecision + 1);

    while (!inputString.isEmpty() && inputString.endsWith('0')) {
        inputString.chop(1);
    }
    if (inputString.endsWith(QLocale::system().decimalPoint())) {
        inputString.chop(1);
    }

    ProviderResult *result = new ProviderResult;
    result->icon = "exchange-positions";
    result->object = this;
    result->name = i18nc("conversion from one unit to another", "%1 %2 is %3 %4", inputString, inputValue.unit().symbol(), outputString, outputValue.unit().symbol());
    result->program = outputString;
    result->completion = result->name;
    result->type = i18n("Unit conversion");
    result->isCalculation = true;
    list.append(result);

    return list;
}

int Units::launch(const QString &exec)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(exec);
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

    KUnitConversion::Unit fallback;
    static const QRegularExpression nonAlphaRegex("([^A-Za-z]+)");
    QString simpleName = name;
    simpleName.remove(nonAlphaRegex);

    for (const QString &candidateName : category.allUnits()) {
        QString simpleCandidateName = candidateName.toLower();
        simpleCandidateName.remove(nonAlphaRegex);
        if (simpleCandidateName.startsWith(simpleName)) { // TODO: return multiple matches
            fallback = category.unit(candidateName);
        }

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

    return fallback;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
