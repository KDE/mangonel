/*
 * lexer.ll - part of abakus
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
%option noyywrap
%{
#define YY_NO_UNPUT

#include <QDebug>

#include "node.h"
#include "function.h"
#include "parser_yacc.hpp"
#include "result.h"

int yyCurTokenPos;
int yyThisTokenLength;

int yyparse(void);
%}

DIGITS [0-9]+
HEX [0-9A-Fa-f]+
%%

 /* Always skip whitespace */
[ \t]* { yyCurTokenPos += yyThisTokenLength; yyThisTokenLength = yyleng; }

 /* Power operator */
"**" { 
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 2;
    return POWER;
}

"^" {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 1;
    return POWER;
}

[sS][eE][tT] {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 3;
    return SET;
}

[rR][eE][mM][oO][vV][eE] {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 6;
    return REMOVE;
}

[dD][eE][rR][iI][vV] {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 5;
    return DERIV;
}

 /* Read numbers of the form with at least the decimal point and trailing
 * digits, such as .32, -234.45, .0, etc.  Numbers are only read in the BEGIN
 * state.
 */
{DIGITS}*([\.,]{DIGITS}+)(e[-+]?{DIGITS}+)? {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;
    return NUM;
}

 /* Read Hex */
0x({HEX}+)? {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;
    return NUM;
}

 /* Read numbers with at least the integral part, such as +4234, -34e8, etc.
 * Numbers are only read in the BEGIN state.
 */
{DIGITS}+([\.,]{DIGITS}*)?(e[-+]?{DIGITS}+)? {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;
    return NUM;
}

[nN][aA][nN] {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;
    return NUM;
}

[iI][nN][fF] {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;
    return NUM;
}

 /* This detects textual input, and if it isn't pre-declared by the parser (in
 * other words, if it isn't a function), then it is returned as an identifier.
 */
[a-zA-Z_][a-zA-Z_0-9]* {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = yyleng;

    if(FunctionManager::instance()->isFunction(yytext))
        return FN;
    else {
        return ID;
    }
}

 /* All other characters are returned as-is to the parser, who can accept or
 * reject it as needed.
 */
. {
    yyCurTokenPos += yyThisTokenLength;
    yyThisTokenLength = 1;
    return *yytext;
}

%%

class Lexer::Private
{
public:
    YY_BUFFER_STATE buffer;
    int lastToken, thisToken;
    int lastPos, thisPos;
    QString lastTokenData, thisTokenData;
};

/* Declared in function.h, implemented here in lexer.l since this is where
 * all the yy_*() functions and types are defined.
 */
Lexer::Lexer(const QString &expr) :
    m_private(new Private)
{
    QByteArray exprString = expr.toLatin1();

    yyCurTokenPos = 0;
    yyThisTokenLength = 0;

    m_private->buffer = yy_scan_string(exprString.data() ? exprString.data() : "");
    m_private->lastToken = -1;
    m_private->lastPos = -1;

    m_private->thisToken = yylex();
    m_private->thisTokenData = QString(yytext);

    if(yyCurTokenPos != 0)
    {
        qWarning() << "yyCurTokenPos should be 0!!\n";
    }

    m_private->thisPos = yyCurTokenPos;
}

Lexer::~Lexer()
{
    yy_delete_buffer(m_private->buffer);
    delete m_private;
}

bool Lexer::hasNext() const
{
    return m_private->thisToken > 0;
}

int Lexer::nextType()
{
    m_private->lastTokenData = m_private->thisTokenData;
    m_private->lastPos = m_private->thisPos;
    m_private->lastToken = m_private->thisToken;

    m_private->thisToken = yylex();
    m_private->thisTokenData = QString(yytext);
    m_private->thisPos = yyCurTokenPos;

    return m_private->lastToken;
}

QString Lexer::tokenValue() const
{
    return m_private->lastTokenData;
}

int Lexer::tokenPos() const
{
    return m_private->lastPos;
}

/* Declared in function.h, implemented here in lexer.l since this is where
 * all the yy_*() functions and types are defined.
 */
Abakus::number_t parseString(const char *str)
{
    YY_BUFFER_STATE buffer = yy_scan_string(str);

    yyCurTokenPos = 0;
    yyThisTokenLength = 0;

    yyparse();
    yy_delete_buffer(buffer);

    if(Result::lastResult()->type() != Result::Value)
        return Abakus::number_t();

    return Result::lastResult()->result()->value();
}

/* vim: set et sw=4 ts=8: */
