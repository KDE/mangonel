#include "Calculator.h"

#include <assert.h>
#include <math.h>
#include <QApplication>
#include <QClipboard>
#include <QStringList>


const QList<char> operators = (QList<char>() << '+' << '-' << '/' << '*' << '^' << '%');
bool succes = false;


Calculator::Calculator()
{
    functions['+'] = &add;
    functions['-'] = &substract;
    functions['/'] = &divide;
    functions['*'] = &multiply;
    functions['^'] = &power;
    functions['%'] = &modulo;

    testCalc();
}

Calculator::~Calculator()
{}

QList<Application> Calculator::getResults(QString query)
{
    succes = false;
    QList<Application> list = QList<Application>();
    float awnser = calculate(query);
    if (succes)
    {
        Application result = Application();
        result.icon = "accessories-calculator";
        result.name = QString::number(awnser, 'g', 12);
        result.program = result.name;
        result.object = this;
        list.append(result);
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
            if (not operators.contains(query.at(index-1).toAscii()))
            {
                oper = item;
                values = query.split(item);
                break;
            }
        }
    }
    if (values.isEmpty())
        return query.toFloat(&succes);
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


namespace
{

float add(float val1, float val2)
{
    return val1 + val2;
}

float substract(float val1, float val2)
{
    return val1 - val2;
}

float divide(float val1, float val2)
{
    return val1 / val2;
}

float multiply(float val1, float val2)
{
    return val1 * val2;
}
float power(float val1, float val2)
{
    return pow(val1, val2);
}
float modulo(float val1, float val2)
{
    return fmod(val1, val2);
}
};


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


#include "Calculator.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4;
