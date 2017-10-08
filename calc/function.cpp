/*
 * function.cpp - part of abakus
 * Copyright (C) 2004, 2005 Michael Pyne <michael.pyne@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "numerictypes.h"

#include <QDebug>

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QRegExp>

#include <math.h>

#include "function.h"
#include "node.h"
#include "valuemanager.h"
#include "hmath.h"

// Used to try and avoid recursive function definitions
class DupFinder : public NodeFunctor
{
    public:
    DupFinder(const QString &nameToFind) :
        m_name(nameToFind), m_valid(true)
    {
    }

    virtual ~DupFinder() { }

    bool isValid() const { return m_valid; }

    virtual void operator()(const Node *node)
    {
        if(!m_valid)
            return;

        const BaseFunction *fn = dynamic_cast<const BaseFunction *>(node);
        if(fn && fn->name() == m_name)
            m_valid = false; // Duplicate detected
    }

    private:
    QString m_name;
    bool m_valid;
};

// Define static member for FunctionManager
FunctionManager *FunctionManager::m_manager = 0;

FunctionManager *FunctionManager::instance()
{
    if(!m_manager)
        m_manager = new FunctionManager;

    return m_manager;
}

FunctionManager::FunctionManager(QObject *parent) :
    QObject(parent)
{
    setObjectName("FunctionManager");
}

FunctionManager::~FunctionManager()
{
    qDeleteAll(m_dict);
    m_dict.clear();
}

// Dummy return value to enable static initialization in the DECL_*()
// macros.
bool FunctionManager::addFunction(const QString &name, function_t fn, const QString &desc)
{
    Function *newFn = new Function;
    QRegExp returnTrigRE("^a(cos|sin|tan)");
    QRegExp needsTrigRE("^(cos|sin|tan)");
    QString fnName(name);

    newFn->name = name;
    newFn->description = desc;
    newFn->fn = fn;
    newFn->userDefined = false;
    newFn->returnsTrig = fnName.contains(returnTrigRE);
    newFn->needsTrig = fnName.contains(needsTrigRE);

    m_dict.insert(name, newFn);

    return false;
}

#define DECLARE_FUNC(name, fn, desc) bool dummy##name = FunctionManager::instance()->addFunction(#name, fn, desc)

// Declares a function name that is implemented by the function of a different
// name. e.g. atan -> Abakus::number_t::arctan()
#define DECLARE_FUNC2(name, fnName, desc) DECLARE_FUNC(name, &Abakus::number_t::fnName, desc)

// Declares a function name that is implemented by the function of the
// same base name.
#define DECLARE_FUNC1(name, desc) DECLARE_FUNC2(name, name, desc)

DECLARE_FUNC1(sin, "Trigonometric sine");
DECLARE_FUNC1(cos, "Trigonometric cosine");
DECLARE_FUNC1(tan, "Trigonometric tangent");

DECLARE_FUNC1(sinh, "Hyperbolic sine");
DECLARE_FUNC1(cosh, "Hyperbolic cosine");
DECLARE_FUNC1(tanh, "Hyperbolic tangent");

DECLARE_FUNC1(atan, "Inverse tangent");
DECLARE_FUNC1(acos, "Inverse cosine");
DECLARE_FUNC1(asin, "Inverse sine");

DECLARE_FUNC1(asinh, "Inverse hyperbolic sine");
DECLARE_FUNC1(acosh, "Inverse hyperbolic cosine");
DECLARE_FUNC1(atanh, "Inverse hyperbolic tangent");

DECLARE_FUNC1(abs, "Absolute value of number");
DECLARE_FUNC1(sqrt, "Square root");
DECLARE_FUNC1(ln, "Natural logarithm (base e)");
DECLARE_FUNC1(log, "Logarithm (base 10)");
DECLARE_FUNC1(exp, "Natural exponential function");

DECLARE_FUNC1(round, "Round to nearest number");
DECLARE_FUNC1(ceil, "Nearest greatest integer");
DECLARE_FUNC1(floor, "Nearest lesser integer");
DECLARE_FUNC2(int, integer, "Integral part of number");
DECLARE_FUNC1(frac, "Fractional part of number");

Function *FunctionManager::function(const QString &name)
{
    if(!m_dict.contains(name))
        return 0;

    return m_dict.value(name, 0);
}

// Returns true if the named identifier is a function, false otherwise.
bool FunctionManager::isFunction(const QString &name)
{
    return function(name) != 0;
}

bool FunctionManager::isFunctionUserDefined(const QString &name)
{
    const Function *fn = function(name);
    return (fn != 0) && (fn->userDefined);
}

bool FunctionManager::addFunction(BaseFunction *fn, const QString &dependantVar)
{
    // First see if this function is recursive
    DupFinder dupFinder(fn->name());
    UnaryFunction *unFunction = dynamic_cast<UnaryFunction *>(fn);
    if(unFunction && unFunction->operand()) {
        unFunction->operand()->applyMap(dupFinder);
        if(!dupFinder.isValid())
            return false;
    }

    // Structure holds extra data needed to call the user defined
    // function.
    UserFunction *newFn = new UserFunction;
    newFn->sequenceNumber = m_dict.count();
    newFn->fn = fn;
    newFn->varName = QString(dependantVar);

    // Now setup the Function data structure that holds the information
    // we need to access and call the function later.
    Function *fnTabEntry = new Function;
    fnTabEntry->name = fn->name();
    fnTabEntry->userFn = newFn;
    fnTabEntry->returnsTrig = false;
    fnTabEntry->needsTrig = false;
    fnTabEntry->userDefined = true;

    if(m_dict.contains(fn->name()))
        emit signalFunctionRemoved(fn->name());

    m_dict.insert(fn->name(), fnTabEntry);
    emit signalFunctionAdded(fn->name());

    return true;
}

void FunctionManager::removeFunction(const QString &name)
{
    Function *fn = function(name);

    // If we remove a function, we need to decrement the sequenceNumber of
    // functions after this one.
    if(fn && fn->userDefined) {
        int savedSeqNum = fn->userFn->sequenceNumber;

        // Emit before we actually remove it so that the info on the function
        // can still be looked up.
        emit signalFunctionRemoved(name);

        delete fn->userFn;
        fn->userFn = 0;

        delete fn;
        m_dict.remove(name);

        foreach(Function *fn, m_dict) {
            UserFunction *userFn = fn->userDefined ? fn->userFn : 0;
            if(userFn && userFn->sequenceNumber > savedSeqNum)
                --fn->userFn->sequenceNumber;
        }
    }
}

QStringList FunctionManager::functionList(FunctionManager::FunctionType type)
{
    functionDict::const_iterator it(m_dict.constBegin());
    QStringList functions;

    switch(type) {
        case Builtin:
            while(it != m_dict.constEnd()) {
                if(!it.value()->userDefined)
                    functions += it.value()->name;
                ++it;
            }
        break;

        case UserDefined:
            // We want to return the function names in the order they were
            // added.
            {
                QVector<Function *> fnTable(m_dict.count(), 0);
                QVector<int> sequenceNumberTable(m_dict.count(), -1);

                // First find out what sequence numbers we have.
                while(it != m_dict.constEnd()) {
                    if(it.value()->userDefined) {
                        int id = it.value()->userFn->sequenceNumber;
                        fnTable[id] = it.value();
                        sequenceNumberTable.append(id);
                    }

                    ++it;
                }

                // Now sort the sequence numbers and return the ordered list
                qSort(sequenceNumberTable.begin(), sequenceNumberTable.end());

                foreach(int sequenceNumber, sequenceNumberTable) {
                    if(sequenceNumber >= 0)
                        functions += fnTable[sequenceNumber]->name;
                }
            }
        break;

        case All:
            functions += functionList(Builtin);
            functions += functionList(UserDefined);
        break;
    }

    return functions;
}

// Applies the function identified by func, using value as a parameter.
Abakus::number_t evaluateFunction(const Function *func, const Abakus::number_t value)
{
    if(func->userDefined) {
        // Pull real entry from userFunctionTable
        UserFunction *realFunction = func->userFn;

        bool wasSet = ValueManager::instance()->isValueSet(realFunction->varName);
        Abakus::number_t oldValue;
        if(wasSet)
            oldValue = ValueManager::instance()->value(realFunction->varName);

        ValueManager::instance()->setValue(realFunction->varName, value);
        Abakus::number_t result = realFunction->fn->value();

        if(wasSet)
            ValueManager::instance()->setValue(realFunction->varName, oldValue);
        else
            ValueManager::instance()->removeValue(realFunction->varName);

        return result;
    }

    return (value.*(func->fn))();
}

void setTrigMode(Abakus::TrigMode mode)
{
    Abakus::m_trigMode = mode;
}

Abakus::TrigMode trigMode()
{
    return Abakus::m_trigMode;
}

// vim: set et sw=4 ts=8:
