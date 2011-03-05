#ifndef Calculator_H
#define Calculator_H

#include "Provider.h"

typedef float (*calcFunct)(float val1, float val2);

class Calculator : public Provider
{
    Q_OBJECT
public:
    Calculator();
    ~Calculator();
public slots:
    QList<Application> getResults(QString query);
    int launch(QVariant selected);
private:
    QHash<char, calcFunct> functions;
    float calculate(QString query);

    void testCalc();
};

namespace
{
float add(float val1, float val2);
float substract(float val1, float val2);
float divide(float val1, float val2);
float multiply(float val1, float val2);
float power(float val1, float val2);
float modulo(float val1, float val2);
}

#endif // Calculator_H

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
