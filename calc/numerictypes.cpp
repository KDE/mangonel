/*
 * numerictypes.cpp - part of abakus
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
#include "hmath.h"

#include <QDebug>
#include <QLocale>

Abakus::TrigMode Abakus::m_trigMode = Abakus::Degrees;
int Abakus::m_prec = -1;

// Converts hmath number to a string.

namespace Abakus
{

QString convertToString(const HNumber &num)
{
    QString str = HMath::formatGenString(num, m_prec);
    QString decimalSymbol = QLocale::system().decimalPoint();
    str.replace('.', decimalSymbol);

    QStringList parts = str.split('e');
    QRegExp zeroKiller("(" + QRegExp::escape(decimalSymbol) +
                       "\\d*[1-9])0*$"); // Remove trailing zeroes.
    QRegExp zeroKiller2("(" + QRegExp::escape(decimalSymbol) + ")0*$");

    str = parts[0];
    str.replace(zeroKiller, "\\1");
    str.replace(zeroKiller2, "\\1");
    if(str.endsWith(decimalSymbol))
        str.truncate(str.length() - 1); // Remove trailing period.

    if(parts.count() > 1 && parts[1] != "0")
        str += QString("e%1").arg(parts[1]);

    return str;
}

} // namespace Abakus.

const Abakus::number_t::value_type Abakus::number_t::PI = HMath::pi();
const Abakus::number_t::value_type Abakus::number_t::E = HMath::exp(1);

// vim: set et ts=8 sw=4:
