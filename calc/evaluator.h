/* This file was part of the SpeedCrunch project
   Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>

   And is now part of abakus.
   Copyright (c) 2005 Michael Pyne <michael.pyne@kdemail.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef ABAKUS_EVALUATOR_H
#define ABAKUS_EVALUATOR_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "numerictypes.h"

class Token
{
public:
    typedef enum
    {
      Unknown,
      Number,
      Operator,
      Identifier
    } Type;

    typedef enum
    {
      InvalidOp = 0,
      Plus,           //  + (addition)
      Minus,          //  - (substraction, negation)
      Asterisk,       //  * (multiplication)
      Slash,          //  / (division)
      Caret,          //  ^ (power) or **.
      LeftPar,        //  (
      RightPar,       //  )
      Comma,          // argument separator
      Percent,
      Equal           // variable assignment
    } Op;

    Token( Type type = Unknown, const QString& text = QString(), int pos = -1 );

    Token( const Token& );
    Token& operator=( const Token& );

    Type type() const { return m_type; }
    QString text() const { return m_text; }
    int pos() const { return m_pos; };

    bool isNumber() const { return m_type == Number; }
    bool isOperator() const { return m_type == Operator; }
    bool isIdentifier() const { return m_type == Identifier; }

    Abakus::number_t asNumber() const;
    Op asOperator() const;

    QString description() const;

    static const Token null;

protected:
    Type m_type;
    QString m_text;
    int m_pos;
};


class Tokens: public QVector<Token>
{
public:
    Tokens(): QVector<Token>(), m_valid(true) {};

    bool valid() const { return m_valid; }
    void setValid( bool v ) { m_valid = v; }

protected:
    bool m_valid;
};

class Variable
{
public:
    QString name;
    Abakus::number_t value;
};

class Evaluator
{
public:
    Evaluator();
    ~Evaluator();

    void setExpression( const QString& expr );
    QString expression() const;

    void clear();
    bool isValid() const;

    Tokens tokens() const;
    static Tokens scan( const QString& expr );

    QString error() const;

    // Abakus::number_t eval();

    static QString autoFix( const QString& expr );

private:
    Evaluator( const Evaluator& );
    Evaluator& operator=( const Evaluator& );
};


#endif // EVALUATOR

// vim: set et ts=8 sw=4:
