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

#ifndef Provider_H
#define Provider_H

#include <QObject>
#include <QVariant>

struct Popularity
{
    qint64 lastUse;
    qint64 count;
    QStringList matchStrings;
};

struct ProviderResult;

// Abstract base class of providers.
class Provider : public QObject
{
    Q_OBJECT

public:
    explicit Provider (QObject *parent);
    Provider() = delete;

    virtual ~Provider() {}

public slots:
    virtual QList<ProviderResult*> getResults(QString query) = 0;
    virtual int launch(const QString &exec) = 0;
};

// Struct stored in AppList.
struct ProviderResult : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString completion MEMBER completion CONSTANT)
    Q_PROPERTY(QString icon MEMBER icon CONSTANT)
    Q_PROPERTY(QString type MEMBER type CONSTANT)
    Q_PROPERTY(int priority MEMBER priority CONSTANT)

public:
    bool isCalculation = false;
    QString name;
    QString completion;
    QString icon;
    long priority = 0;//INT_MAX;
    QString program;
    Provider *object{}; //Pointer to the search provider that provided this result.
    QString type;

public slots:
    void launch() {
        object->launch(program);
    }
};

#endif // Provider_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
