/*
 * parser.yy - part of abakus
 * Copyright (C) 2004, 2005, 2008 Michael Pyne <michael.pyne@kdemail.net>
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
%{

#define QT_NO_ASCII_CAST
#include <QtCore/QString>

/* Add necessary includes here. */
#include <QDebug>

#include <klocalizedstring.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "result.h"
#include "node.h"
#include "function.h"
#include "valuemanager.h"

extern char *yytext;

extern int gCheckIdents;

int yylex(void);
int yyerror(const char *);

%}

%union {
    Node *node;
    NumericValue *value;
    UnaryFunction *fn;
    Identifier *ident;
}

%token <value> NUM
%type <node> EXP FACTOR TERM S NUMBER VALUE FINAL
%type <fn> FUNC
%token <fn> FN
%token <ident> ID
%type <ident> IDENT ASSIGN
%token POWER "**"
%token SET "set"
%token REMOVE "remove"
%token DERIV "deriv"

%%

/**
 * Parser design:
 *
 * This is pretty standard stuff for the calculator part (read in tokens from
 * the lexer, and form a syntax tree using the Node* objects).  The unusual
 * part is that due to the design of bison, we don't actually return a value
 * normally to the calling function.
 *
 * Instead, we make use of the static Result::setLastResult() call in order
 * to notify the calling function of the result of the parse.  There are
 * different statuses you can set, including Error (with a message), Null
 * (which indicates that some action happened that doesn't generate a result),
 * and Value (with a Node* that holds the result).
 *
 * If you are done parsing before reaching the FINAL token, you can call:
 *     YYACCEPT: Done, parsed successfully.
 *     YYERROR : Done, there was an error.
 *
 * Note that if you let the parse bubble back up to FINAL, then the result
 * will always be a Value.
 */

FINAL: { gCheckIdents = 1; } S {
    Result::setLastResult(NodePtr($2));
    $$ = 0;
}

S: EXP { $$ = $1; }

// Rudimentary error handling
S: error '=' {
    Result::setLastResult(i18n("This is an invalid assignment."));

    YYABORT;
}

// Can't assign to a function.
S: FUNC '=' {
    QString s(i18n("You can't assign to function %1", QString($1->name())));
    Result::setLastResult(s);

    YYABORT;
}

// This is a function prototype.  abakus currently only supports one-argument
// functions.
ASSIGN: '(' { --gCheckIdents; } IDENT ')' '=' {
    $$ = $3;
}

// Blocking a variable with the name deriv is a slight feature regression
// since normally functions and variables with the same name can coexist, but
// I don't want to duplicate code all over the place.
S: SET DERIV {
    QString s(i18n("Function %1 is built-in and cannot be overridden.", QLatin1String("deriv")));
    Result::setLastResult(s);

    YYABORT;
}

S: DERIV '=' {
    QString s(i18n("Function %1 is built-in and cannot be overridden.", QLatin1String("deriv")));
    Result::setLastResult(s);

    YYABORT;
}

S: SET FUNC ASSIGN EXP {
    ++gCheckIdents;

    // We're trying to reassign an already defined function, make sure it's
    // not a built-in.
    QString funcName = $2->name();
    QString ident = $3->name();
    FunctionManager *manager = FunctionManager::instance();

    if(manager->isFunction(funcName) && !manager->isFunctionUserDefined(funcName)) {
        QString s(i18n("Function %1 is built-in and cannot be overridden.", funcName));
        Result::setLastResult(s);

        YYABORT;
    }

    if(manager->isFunction(funcName))
        manager->removeFunction(funcName);

    QByteArray fnName = funcName.toLatin1();
    BaseFunction *newFn = new UserDefinedFunction(fnName.data(), $4);
    if(!manager->addFunction(newFn, ident)) {
        QString s(i18n("Unable to define function %1 because it is recursive.", funcName));
        Result::setLastResult(s);

        YYABORT;
    }

    Result::setLastResult(Result::Null);
    YYACCEPT;
}

// IDENT is the same as FUNC, except that the lexer has determined that IDENT
// is not already a FUNC.
S: SET IDENT ASSIGN EXP {
    ++gCheckIdents;

    QString funcName = $2->name();
    QString ident = $3->name();

    // No need to check if the function is already defined, because the
    // lexer checked for us before returning the IDENT token.
    QByteArray fnName = funcName.toLatin1();
    BaseFunction *newFn = new UserDefinedFunction(fnName.data(), $4);
    FunctionManager::instance()->addFunction(newFn, ident);

    Result::setLastResult(Result::Null);
    YYACCEPT;
}

// Remove a defined function.
S: REMOVE FUNC '(' ')' {
    FunctionManager::instance()->removeFunction($2->name());

    Result::setLastResult(Result::Null);
    YYACCEPT;
}

// Can't remove an ident using remove-func syntax.
S: REMOVE IDENT '(' ')' {
    // This is an error
    Result::setLastResult(Result(i18n("Function %1 is not defined.", QString($2->name()))));
    YYABORT;
}

// This happens when the user tries to remove a function that's not defined.
S: REMOVE IDENT '(' IDENT ')' {
    // This is an error
    Result::setLastResult(Result(i18n("Function %1 is not defined.", QString($2->name()))));
    YYABORT;
}

S: REMOVE IDENT {
    ValueManager *manager = ValueManager::instance();

    if(manager->isValueSet($2->name()) && !manager->isValueReadOnly($2->name())) {
        manager->removeValue($2->name());

        Result::setLastResult(Result::Null);
        YYACCEPT;
    }
    else {
        QString s;
        if(manager->isValueSet($2->name()))
            s = i18n("Can't remove predefined variable %1.", QString($2->name()));
        else
            s = i18n("Can't remove undefined variable %1.", QString($2->name()));

        Result::setLastResult(s);

        YYABORT;
    }
}

S: SET IDENT '=' EXP {
    ValueManager *vm = ValueManager::instance();

    if(vm->isValueReadOnly($2->name())) {
        if($2->name() == "pi" && $4->value() == Abakus::number_t("3.0"))
            Result::setLastResult(i18n("This isn't Indiana, you can't just change pi"));
        else
            Result::setLastResult(i18n("%1 is a constant", QString($2->name())));

        YYABORT;
    }

    ValueManager::instance()->setValue($2->name(), $4->value());

    Result::setLastResult(Result::Null);
    YYACCEPT;
}

// Set a variable.
S: IDENT '=' EXP {
    ValueManager *vm = ValueManager::instance();

    if(vm->isValueReadOnly($1->name())) {
        if($1->name() == "pi" && $3->value() == Abakus::number_t("3.0"))
            Result::setLastResult(i18n("This isn't Indiana, you can't just change pi"));
        else
            Result::setLastResult(i18n("%1 is a constant", QString($1->name())));

        YYABORT;
    }

    ValueManager::instance()->setValue($1->name(), $3->value());

    Result::setLastResult(Result::Null);
    YYACCEPT;
}

S: NUMBER '=' {
    Result::setLastResult(i18n("Can't assign to %1", $1->value().toString()));
    YYABORT;
}

// Can't call this as a function.
TERM: IDENT '(' {
    Result::setLastResult(i18n("%1 isn't a function (or operator expected)", QString($1->name())));
    YYABORT;
}

// Can't do this either.
TERM: IDENT IDENT {
    Result::setLastResult(i18n("Missing operator"));
    YYABORT;
}

TERM: IDENT NUMBER {
    Result::setLastResult(i18n("Missing operator"));
    YYABORT;
}

TERM: NUMBER NUMBER {
    Result::setLastResult(i18n("Missing operator"));
    YYABORT;
}

S: error {
    Result::setLastResult(i18n("Sorry, I can't figure it out."));
    YYABORT;
}

/**
 * Here be the standard calculator-parsing part.  Nothing here should be too
 * fancy.
 */
EXP: EXP '+' FACTOR { $$ = new BinaryOperator(BinaryOperator::Addition, $1, $3); }
EXP: EXP '-' FACTOR { $$ = new BinaryOperator(BinaryOperator::Subtraction, $1, $3); }
EXP: FACTOR { $$ = $1; }

FACTOR: FACTOR '*' TERM { $$ = new BinaryOperator(BinaryOperator::Multiplication, $1, $3); }
FACTOR: FACTOR '/' TERM { $$ = new BinaryOperator(BinaryOperator::Division, $1, $3); }
FACTOR: TERM { $$ = $1; }

/*
 * Handle exponentiation by making them TERMs, which makes the POWER operator bind very
 * tightly.  Make unary negation a lower precendence so that -2^2 == -4, as 2^2 will be
 * reduced to a TERM before the -TERM reduction is applied.
 */
TERM: VALUE POWER TERM { $$ = new BinaryOperator(BinaryOperator::Exponentiation, $1, $3); }
TERM: '+' TERM { $$ = $2; }
TERM: '-' TERM { $$ = new UnaryOperator(UnaryOperator::Negation, $2); }
TERM: '(' EXP ')' { $$ = $2; }

TERM: VALUE { $$ = $1; }

VALUE: NUMBER { $$ = $1; }

NUMBER: NUM {
    QChar decimal = QLocale::system().decimalPoint();

    // Replace current decimal separator with US Decimal separator to be
    // evil.
    unsigned len = strlen(yytext);
    for(unsigned i = 0; i < len; ++i)
        if(yytext[i] == decimal)
            yytext[i] = '.';

    Abakus::number_t value(yytext);

    $$ = new NumericValue(value);
}

TERM: DERIV { --gCheckIdents; } '(' EXP ',' { ++gCheckIdents; } EXP ')' {
    $$ = new DerivativeFunction($4, $7);
}

TERM: FUNC TERM {
    $1->setOperand($2);
    $$ = $1;
}

/* Handle implicit multiplication */
TERM: NUMBER FUNC TERM {
    $2->setOperand($3);
    $$ = new BinaryOperator(BinaryOperator::Multiplication, $1, $2);
}

TERM: NUMBER '(' EXP ')' {
    $$ = new BinaryOperator(BinaryOperator::Multiplication, $1, $3);
}

TERM: NUMBER IDENT {
    if(gCheckIdents > 0 && !ValueManager::instance()->isValueSet($2->name())) {
        Result::setLastResult(i18n("Unknown variable %1", QString($2->name())));
        YYABORT;
    }

    $$ = new BinaryOperator(BinaryOperator::Multiplication, $1, $2);
}

VALUE: IDENT {
    if(gCheckIdents <= 0 || ValueManager::instance()->isValueSet($1->name()))
        $$ = $1;
    else {
        Result::setLastResult(i18n("Unknown variable %1", QString($1->name())));
        YYABORT;
    }
}

IDENT: ID {
    $$ = new Identifier(yytext);
}

FUNC: FN {
    /* No check necessary, the lexer has already checked for us. */
    $$ = new BuiltinFunction(yytext, 0);
}

%%

int gCheckIdents = 0;

int yyerror(const char *)
{
    return 0;
}

// vim: set et ts=8 sw=4:
