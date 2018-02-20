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

#include <iostream>

static const QHash<QChar, calcFunct> s_functions({
        {'+', [](double val1, double val2) { return val1 + val2; }},
        {'-', [](double val1, double val2) { return val1 - val2; }},
        {'/', [](double val1, double val2) { return val1 / val2; }},
        {'*', [](double val1, double val2) { return val1 * val2; }},
        {'^', [](double val1, double val2) { return pow(val1, val2); }},
        {'%', [](double val1, double val2) { return fmod(val1, val2); }}
});

const QList<char> operators = (QList<char>() << '+' << '-' << '/' << '*' << '^' << '%');
bool succes = false;

Calculator::Calculator(QObject *parent) :
    Provider(parent)
{
    testCalc();
}

Calculator::~Calculator()
{}

QList<Application*> Calculator::getResults(QString query)
{
    succes = false;
    QList<Application*> list;
    QString result;

    QRegExp convertPattern("(.+)(?:to|in)\\s+((bin|oct|hex)\\w*)$");
    if (query.contains(convertPattern)) {
        QString target = convertPattern.cap(2);
        long calculated = calculate(convertPattern.cap(1));

        if (target.startsWith("bin")) {
            result = "0b" + QString::number(calculated, 2);
        } else if (target.startsWith("oct")) {
            result = "0" + QString::number(calculated, 8);
        } else if (target.startsWith("hex")) {
            result = "0x" + QString::number(calculated, 16);
        }
    } else {
        int precision = 2;
        double calculationResult = calculate(query);

        if (calculationResult < 100) {
            precision = 6;
        }

        double scaledResult = calculationResult * std::pow(10, precision);
        while (precision > 0 && std::floor(scaledResult) == std::ceil(scaledResult)) {
            precision--;
            scaledResult = calculationResult * std::pow(10, precision);
        }

        result = QLocale::system().toString(calculationResult, 'f', precision + 1);
    }

    if (succes)
    {
        Application *app = new Application;
        app->icon = "accessories-calculator";
        app->name = result;
        app->program = app->name;
        app->object = this;
        app->type = i18n("Calculation");
        list.append(app);
    }
    return list;
}

double Calculator::calculate(QString query)
{
    query.remove(' ');
    if (query.length() <= 0)
        return 0;
    int pos = 0;
    QChar ch = query.at(pos);

    int count = 0;
    QString inner;
    while (pos < query.length()) {
        ch = query.at(pos);
        if (ch == ')')
        {
            count -= 1;
            if (count <= 0)
            {
                QString result = QString::number(calculate(inner), 'f', 12);
                pos -= inner.length() + 2;
                pos += result.length();
                query.replace("("+inner+")", result);
            }
        }
        if (count > 0)
                inner += ch;
        if (ch == '(') {
            count += 1;
        }
        pos += 1;
    }
    QChar oper = ' ';
    QStringList values;

    for(const QChar op : operators) {
        int index = query.indexOf(op);

        if (index == 0) {
            index = query.indexOf(op, 1);
        }

        if (index < 0) {
            continue;
        }

        if (!s_functions.contains(query.at(index-1))) {
            oper = op;
            values = query.split(op);
            break;
        }
    }

    if (values.isEmpty()) {
        // Need to handle binary manually
        if (query.startsWith("0b")) {
            qDebug() << "Was binary" << query << query.mid(2) << query.mid(2).toInt(nullptr, 2);
            return query.mid(2).toInt(nullptr, 2);
        }

        double dec = query.toDouble(&succes);
        if (succes) {
            return dec;
        }
        int integer = query.toInt(&succes, 0);
        return integer;
    }
    if (!s_functions.contains(oper)) {
        return 0 ;
    }

    if (values[0] == "") {
        values.removeFirst();
        values[0] = oper + values[0];
    }
    double value1 = calculate(values.takeFirst());
    double value2 = calculate(values.join(QString(oper)));
    return s_functions[oper](value1, value2);

    return 0;
}

int Calculator::launch(QVariant selected)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(selected.toString(), QClipboard::Selection);
    return 0;
}


static void equalsAssert(const char *expr, double expected, const char *what)
{
    const double calculated = Calculator::calculate(expr);
    if (!qFuzzyCompare(calculated, expected)) {
        std::cerr << what << std::endl;
        std::cerr << expr << "=" << calculated << " should be " << expected << std::endl;
    }
}

static void precisionAssert(const char *expr, double subtract, double maximum, const char *what)
{
    const double calculated = fabs(Calculator::calculate(expr) - subtract);
    if (calculated  > maximum) {
        std::cerr << what << std::endl;
        std::cerr << expr << "=" << calculated << "s hould be less than " << maximum << std::endl;
    }
}

static void notEqualsAssert(const char *expr, double expected, const char *what)
{
    const double calculated = Calculator::calculate(expr);
    if (calculated == expected) {
        std::cerr << what << std::endl;
        std::cerr << expr << "=" << calculated << " should not be " << expected << std::endl;
    }
}

void Calculator::testCalc()
{
    equalsAssert((""), 0,"Test for default output.");
    equalsAssert((" "), 0,"Test for default output.");
    equalsAssert(("23+23"), 46,"Test addition of integers.");
    equalsAssert(("23+-23"), 0,"Test addition of integers.");
    equalsAssert(("34.442+23.558"), 58,"Test addition of double.");
    precisionAssert("-34.442+23.442", -11, 0.00001, "Test addition of double.");
    equalsAssert(("36-7"), 36-7,"Test substraction of integers.");
    equalsAssert(("23.534-12.034"), 11.5,"Test substraction of double.");
    precisionAssert("23.534--12.034", 35.568, 0.000001,"Test substraction of double.");
    equalsAssert(("23/2"), 11.5,"Test division of integers.");
    equalsAssert(("23/-2"), -11.5,"Test division of integers.");
    equalsAssert(("12.5/2"), 6.25,"Test division of a double.");
    equalsAssert(("25*25"), 25*25,"Test multiplication of integers.");
    equalsAssert(("-25*25"), -25*25,"Test multiplication of integers.");
    precisionAssert("-25.3*25.4", -642.62, 0.00001, "precision");
    equalsAssert(("2.5*3.5"), 8.75,"Test multiplication of double.");
    equalsAssert(("2.5*-3.5"), -8.75,"Test multiplication of double.");
    equalsAssert(("2^5"), 32,"Test raising to power of integers.");
    precisionAssert("3.5^2.1", 13.88490, 0.00001,"Test raising to power of double.");
    equalsAssert(("12/0"), 1/0.0,"Test infinity.");
    notEqualsAssert(("0/0.0"), calculate("0/0.0"),"Test NaN.");
    equalsAssert(("443*(43+3)"), 20378,"Test for nesting without testing operator precedence.");
    equalsAssert(("(443+43)*3"), 1458,"Test for nesting with conflicting operator precedence.");
    equalsAssert(("443+43*3"), 572,"Test operator precedence.");
    equalsAssert(("23*((23-3)*34+43)"), 16629, "groups");
    equalsAssert(("23*(84+(-23-3)*34+-43)"), -19389, "groups");
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
