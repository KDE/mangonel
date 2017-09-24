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


const QList<char> operators = (QList<char>() << '+' << '-' << '/' << '*' << '^' << '%');
bool succes = false;

Calculator::Calculator()
{
    functions['+'] = [](float val1, float val2) { return val1 + val2; };
    functions['-'] = [](float val1, float val2) { return val1 - val2; };
    functions['/'] = [](float val1, float val2) { return val1 / val2; };
    functions['*'] = [](float val1, float val2) { return val1 * val2; };
    functions['^'] = [](float val1, float val2) { return pow(val1, val2); };
    functions['%'] = [](float val1, float val2) { return fmod(val1, val2); };

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
        result = QString::number(calculate(query), 'g', 12);
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

float Calculator::calculate(QString query)
{
    query.remove(' ');
    if (query.length() <= 0)
        return 0;
    int pos = 0;
    QChar ch = query.at(pos);
    int count = 0;
    QString inner = "";
    while (pos < query.length())
    {
        ch = query.at(pos);
        if (ch == ')')
        {
            count -= 1;
            if (count <= 0)
            {
                QString result = QString::number(this->calculate(inner), 'f', 12);
                pos -= inner.length() + 2;
                pos += result.length();
                query.replace("("+inner+")", result);
            }
        }
        if (count > 0)
                inner += ch;
        if (ch == '(')
        {
            count += 1;
        }
        pos += 1;
    }
    char oper = ' ';
    QStringList values;
    foreach(char item, operators)
    {
        int index = query.indexOf(item);
        if (index == 0)
            index = query.indexOf(item, 1);
        if (index > 0)
        {
            if (not operators.contains(query.at(index-1).toLatin1()))
            {
                oper = item;
                values = query.split(item);
                break;
            }
        }
    }
    if (values.isEmpty()) {
        float dec = query.toFloat(&succes);
        if (succes) {
            return dec;
        }
        int integer = query.toInt(&succes, 0);
        return integer;
    }
    if (functions.contains(oper))
    {
        if (values[0] == "")
        {
            values.removeFirst();
            values[0] = oper + values[0];
        }
        float value1 = calculate(values.takeFirst());
        float value2 = calculate(values.join(QString(oper)));
        return functions[oper](value1, value2);
    }
    else
        succes = true;
        return 0;
}

int Calculator::launch(QVariant selected)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(selected.toString(), QClipboard::Selection);
    return 0;
}


void Calculator::testCalc()
{
    assert(calculate("") == 0); // Test for default output.
    assert(calculate(" ") == 0); // Test for default output.
    assert(calculate("23+23") == 46); // Test addition of integers.
    assert(calculate("23+-23") == 0); // Test addition of integers.
    assert(calculate("34.442+23.558") == 58); // Test addition of floats.
    assert(fabs(calculate("-34.442+23.442") - -11) <= 0.00001); // Test addition of floats.
    assert(calculate("36-7") == 36-7); // Test substraction of integers.
    assert(calculate("23.534-12.034") == 11.5); // Test substraction of floats.
    assert(fabs(calculate("23.534--12.034") - 35.568) <= 0.000001); // Test substraction of floats.
    assert(calculate("23/2") == 11.5); // Test division of integers.
    assert(calculate("23/-2") == -11.5); // Test division of integers.
    assert(calculate("12.5/2") == 6.25); // Test division of a float.
    assert(calculate("25*25") == 25*25); // Test multiplication of integers.
    assert(calculate("-25*25") == -25*25); // Test multiplication of integers.
    assert(fabs(calculate("-25.3*25.4") - -642.62) <= 0.00001);
    assert(calculate("2.5*3.5") == 8.75); // Test multiplication of floats.
    assert(calculate("2.5*-3.5") == -8.75); // Test multiplication of floats.
    assert(calculate("2^5") == 32); // Test raising to power of integers.
    assert(fabs(calculate("3.5^2.1") - 13.88490) <= 0.00001); // Test raising to power of floats.
    assert(calculate("12/0") == 1/0.0); // Test infinity.
    assert(calculate("0/0.0") != calculate("0/0.0")); // Test NaN.
    assert(calculate("443*(43+3)") == 20378); // Test for nesting without testing operator precedence.
    assert(calculate("(443+43)*3") == 1458); // Test for nesting with conflicting operator precedence.
    assert(calculate("443+43*3") == 572); // Test operator precedence.
    assert(calculate("23*((23-3)*34+43)") == 16629);
    assert(calculate("23*(84+(-23-3)*34+-43)") == -19389);
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
