/*
 * Copyright 2010-2012 Bart Kroon <bart@tarmack.eu>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Units_H
#define Units_H

#include "Provider.h"
#include <KUnitConversion/Converter>

class QThread;
class QTimer;

class CurrencyRefresher : public QObject
{
    Q_OBJECT

public slots:
    void refresh();
    void init();

private:
    QTimer *m_timer;
};

class Units : public Provider
{
    Q_OBJECT
public:
    Units(QObject *parent);
    ~Units();
public slots:
    QList<ProviderResult*> getResults(QString query) override;
    int launch(const QString &exec) override;

private:
    enum UnitMatchingLevel {
        OnlyCommonUnits,
        AllUnits
    };
    KUnitConversion::Unit resolveUnitName(const QString &name, const KUnitConversion::UnitCategory &category = KUnitConversion::UnitCategory());
    KUnitConversion::Unit matchUnitCaseInsensitive(const QString &name, const KUnitConversion::UnitCategory &category, const UnitMatchingLevel level);

    KUnitConversion::Converter m_converter;

    CurrencyRefresher *m_refresher;
    QThread *m_refresherThread;
};
    
    

#endif // Units_H
// kate: indent-mode cstyle; space-indent on; indent-width 4;
