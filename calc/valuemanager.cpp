/*
 * valuemanager.cpp - part of abakus
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
#include <QDebug>
#include <klocalizedstring.h>

#include <QtCore/QRegExp>

#include "numerictypes.h"
#include "valuemanager.h"

ValueManager *ValueManager::m_manager = 0;

ValueManager *ValueManager::instance()
{
    if(!m_manager)
        m_manager = new ValueManager;

    return m_manager;
}

ValueManager::ValueManager(QObject *parent) : QObject(parent)
{
    m_values.insert("pi", Abakus::number_t::PI);
    m_values.insert("e", Abakus::number_t::E);

    setObjectName ("ValueManager");
}

Abakus::number_t ValueManager::value(const QString &name) const
{
    return m_values[name];
}

bool ValueManager::isValueSet(const QString &name) const
{
    return m_values.contains(name);
}

bool ValueManager::isValueReadOnly(const QString &name) const
{
    QRegExp readOnlyValues("^(ans|pi|e|stackCount)$");

    return readOnlyValues.indexIn(name) != -1;
}

void ValueManager::setValue(const QString &name, const Abakus::number_t value)
{
    if(m_values.contains(name) && this->value(name) != value)
        emit signalValueChanged(name, value);
    else if(!m_values.contains(name))
        emit signalValueAdded(name, value);

    m_values.insert(name, value);
}

void ValueManager::removeValue(const QString &name)
{
    if(m_values.contains(name))
        emit signalValueRemoved(name);

    m_values.remove(name);
}

void ValueManager::slotRemoveUserVariables()
{
    QStringList vars = valueNames();

    foreach(QString var, vars)
        if(!isValueReadOnly(var))
            removeValue(var);
}

QStringList ValueManager::valueNames() const
{
    return m_values.keys();
}

QString ValueManager::description(const QString &valueName)
{
    if(valueName == "e")
        return i18n("Natural exponential base - 2.7182818");
    if(valueName == "pi")
        return i18n("pi (π) - 3.1415926");

    return QString();
}

// vim: set et ts=8 sw=4 encoding=utf-8:
