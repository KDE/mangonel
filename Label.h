/*
 * Copyright 2010-2012 Bart Kroon <bart@tarmack.eu>
 * Copyright 2012, 2013 Martin Sandsmark <martin.sandsmark@kde.org>
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

#ifndef Label_H
#define Label_H

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>


class Label : public QLabel
{
    Q_OBJECT

public:
    Label(QWidget* parent = 0);
    ~Label();
    void setText(QString text);
    void appendText(QString text);
    QString completion();
    void setCompletion(QString string);
    QString preEdit();
    void setPreEdit(QString preEdit);
    void paintEvent(QPaintEvent*);

signals:
    void textChanged(QString text);
    
private:
    int paintText(QChar ch, QPen pen);
    QPen makeGradient(QPen pen);
    
    QString m_completionText;
    QString m_preEditText;
    QPen m_frontPen;
    QPen m_backPen;
    QPen m_preEditPen;
    int m_position;
    QPainter* m_painter;

};

namespace
{
int max(int i1, int i2);
QString longest(QString str1, QString str2);
};

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
