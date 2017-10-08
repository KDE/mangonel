#ifndef ABAKUS_VALUEMANAGER_H
#define ABAKUS_VALUEMANAGER_H
/*
 * valuemanager.h - part of abakus
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

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "numerictypes.h"

class ValueManager : public QObject
{
    Q_OBJECT
    public:
    typedef QMap<QString, Abakus::number_t> valueMap;

    static ValueManager *instance();

    Abakus::number_t value(const QString &name) const;

    bool isValueSet(const QString &name) const;
    bool isValueReadOnly(const QString &name) const;

    void setValue(const QString &name, const Abakus::number_t value);
    void removeValue(const QString &name);

    QStringList valueNames() const;

    /**
     * Returns a textual description of a constant built-into abakus.
     */
    static QString description(const QString &valueName);

    signals:
    void signalValueAdded(const QString &name, Abakus::number_t value);
    void signalValueRemoved(const QString &name);
    void signalValueChanged(const QString &name, Abakus::number_t newValue);

    public slots:
    void slotRemoveUserVariables();

    private:
    ValueManager(QObject *parent = 0);

    static ValueManager *m_manager;
    valueMap m_values;
};

#endif

// vim: set et sw=4 ts=8:
