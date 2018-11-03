// This file is part of the SpeedCrunch project
// Copyright (C) 2013 @heldercorreia
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef CORE_NUMBERFORMATTER_H
#define CORE_NUMBERFORMATTER_H

#include "quantity.h"

#include <QtCore/QString>

struct NumberFormatter {
    static QString format(HNumber &num, int precision = -1) { return format(Quantity(num), precision); }
    static QString format(CNumber &num, int precision = -1) { return format(Quantity(num), precision); }
    static QString format(Quantity, int precision = -1);
};

#endif
