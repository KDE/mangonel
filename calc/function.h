#ifndef ABAKUS_FUNCTION_H
#define ABAKUS_FUNCTION_H
/*
 * function.h - part of abakus
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

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QHash>

class BaseFunction;

struct UserFunction
{
    int sequenceNumber;
    BaseFunction *fn;
    QString varName;
};

// Ugly pointer-to-member typedef ahead
typedef Abakus::number_t (Abakus::number_t::*function_t)() const;

struct Function {
    QString name;
    QString description;

    // A function is either builtin or user defined, this union is
    // used for both cases.
    union {
        function_t fn;       // Builtin.
        UserFunction *userFn; // User defined
    };

    bool returnsTrig;
    bool needsTrig;

    bool userDefined;
};

void setTrigMode(Abakus::TrigMode mode);
Abakus::TrigMode trigMode();

class FunctionManager : public QObject
{
    Q_OBJECT
    public:
    typedef QHash<QString, Function *> functionDict;

    ~FunctionManager();

    static FunctionManager *instance();

    Function *function(const QString &name);

    bool isFunction(const QString &name);
    bool isFunctionUserDefined(const QString &name);

    bool addFunction(BaseFunction *fn, const QString &dependantVar);
    bool addFunction(const QString &name, function_t fn, const QString &desc);
    void removeFunction(const QString &name);

    typedef enum { Builtin, UserDefined, All } FunctionType;

    QStringList functionList(FunctionType type);

    signals:
    void signalFunctionAdded(const QString &name);
    void signalFunctionRemoved(const QString &name);

    private:
    FunctionManager(QObject *parent = 0);

    static FunctionManager *m_manager;
    functionDict m_dict;
};

Abakus::number_t evaluateFunction(const Function *func, const Abakus::number_t value);

// Implemented in lexer.l due to prototype issues.
Abakus::number_t parseString(const char *str);

// Implemented in lexer.l due to prototype issues.
class Lexer
{
public:
    Lexer(const QString &expr);
    ~Lexer();

    bool hasNext() const;
    int nextType();

    int tokenPos() const;

    // Can call this after nextType to find the associated string value of the
    // token.
    QString tokenValue() const;

private:
    class Private;
    Private *m_private;
};

#endif

// vim: set et sw=4 ts=8:
