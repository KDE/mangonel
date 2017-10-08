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

#include "evaluator.h"
#include "function.h"
#include "node.h" // For parser_yacc.hpp below
#include "parser_yacc.hpp"

//#include <QtCore/QCoreApplication>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <QDebug>

//
// Reimplementation of goodies from Evaluator follows.
//

Evaluator::Evaluator()
{
}

Evaluator::~Evaluator()
{
}

void Evaluator::setExpression(const QString &expr)
{
    qWarning() << expr<< " not implemented.\n";
}

QString Evaluator::expression() const
{
    qWarning() << " not implemented.\n";
    return QString();
}

void Evaluator::clear()
{
    qWarning() << " not implemented.\n";
    // Yeah, whatever.
}

bool Evaluator::isValid() const
{
    return true;
}

Tokens Evaluator::tokens() const
{
    qWarning() << " not implemented.\n";
    return Tokens();
}

Tokens Evaluator::scan(const QString &expr)
{
    Lexer l(expr);
    Tokens tokens;

    while(l.hasNext())
    {
        int t = l.nextType();
        Token::Type type = Token::Unknown;

        switch(t)
        {
            case POWER:
            case '*':
            case '(':
            case ')':
            case '-':
            case '+':
            case ',':
            case '=':
                type = Token::Operator;
                break;

            case NUM:
                type = Token::Number;
                break;

            case SET:
            case REMOVE:
            case DERIV:
            case FN:
            case ID:
                type = Token::Identifier;
                break;

            default:
                type = Token::Unknown;
                break;
        }

        tokens.append(Token(type, l.tokenValue(), l.tokenPos()));
    }

    return tokens;
}

QString Evaluator::error() const
{
    qWarning() << " not implemented.\n";
    return "No Error Yet";
}

///
/// ARIYA'S CLASS CODE FOLLOWS
///

// for null token
const Token Token::null;

// helper function: return operator of given token text
// e.g. "*" yields Operator::Asterisk, and so on
static Token::Op matchOperator( const QString& text )
{
  Token::Op result = Token::InvalidOp;

  if( text.length() == 1 )
  {
    QChar p = text[0];
    switch( p.unicode() )
    {
        case '+': result = Token::Plus; break;
        case '-': result = Token::Minus; break;
        case '*': result = Token::Asterisk; break;
        case '/': result = Token::Slash; break;
        case '^': result = Token::Caret; break;
        case ',': result = Token::Comma; break;
        case '(': result = Token::LeftPar; break;
        case ')': result = Token::RightPar; break;
        case '%': result = Token::Percent; break;
        case '=': result = Token::Equal; break;
        default : result = Token::InvalidOp; break;
    }
  }

  if( text.length() == 2 )
  {
    if( text == "**" ) result = Token::Caret;
  }

  return result;
}

// creates a token
Token::Token( Type type, const QString& text, int pos )
{
  m_type = type;
  m_text = text;
  m_pos = pos;
}

// copy constructor
Token::Token( const Token& token )
{
  m_type = token.m_type;
  m_text = token.m_text;
  m_pos = token.m_pos;
}

// assignment operator
Token& Token::operator=( const Token& token )
{
  m_type = token.m_type;
  m_text = token.m_text;
  m_pos = token.m_pos;
  return *this;
}

Abakus::number_t Token::asNumber() const
{
  if( isNumber() ) {
    QByteArray numString(m_text.toLatin1());
    return Abakus::number_t( numString.data() );
  }
  else
    return Abakus::number_t();
}

Token::Op Token::asOperator() const
{
  if( isOperator() ) return matchOperator( m_text );
  else return InvalidOp;
}

QString Token::description() const
{
  QString desc;

  switch (m_type )
  {
    case  Number:     desc = "Number"; break;
    case  Identifier: desc = "Identifier"; break;
    case  Operator:   desc = "Operator"; break;
    default:          desc = "Unknown"; break;
  }

  while( desc.length() < 10 ) desc.prepend( ' ' );
  desc.prepend( "  " );
  desc.prepend( QString::number( m_pos ) );
  desc.append( " : " ).append( m_text );

  return desc;
}


QString Evaluator::autoFix( const QString& expr )
{
  int par = 0;
  QString result;

  // strip off all funny characters
  for( int c = 0; c < expr.length(); c++ )
    if( expr[c] >= QChar(32) )
      result.append( expr[c] );

  // automagically close all parenthesis
  Tokens tokens = Evaluator::scan( result );
  for( int i=0; i<tokens.count(); i++ )
    if( tokens[i].asOperator() == Token::LeftPar ) par++;
    else if( tokens[i].asOperator() == Token::RightPar ) par--;
  for(; par > 0; par-- )
    result.append( ')' );

  // special treatment for simple function
  // e.g. "cos" is regarded as "cos(ans)"
  if( !result.isEmpty() )
  {
    Tokens tokens = Evaluator::scan( result );
    if( (tokens.count() == 1) &&
        FunctionManager::instance()->isFunction(tokens[0].text())
      )
    {
      result.append( "(ans)" );
    }
  }

  return result;
}

// vim: set et ts=8 sw=4:
