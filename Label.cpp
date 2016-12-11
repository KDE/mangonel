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

#include "Label.h"


Label::Label(QWidget* parent)
{
    Q_UNUSED(parent);
    QFont* font = new QFont();
    font->setPointSize(font->pointSize()+6);
    this->setFont(*font);
    delete font;
}

Label::~Label()
{}

void Label::setText(QString text)
{
    text = text.remove('\n');
    if (text != this->text())
    {
        QLabel::setText(text);
        emit textChanged(text);
    }
}

void Label::appendText(QString text)
{
    this->setText(this->text() + text);
}

QString Label::completion()
{
    return this->m_completionText;
}

void Label::setCompletion(QString string)
{
    this->m_completionText = string;
    this->update();
}

QString Label::preEdit()
{
    return this->m_preEditText;
}

void Label::setPreEdit(QString preEdit)
{
    this->m_preEditText = preEdit;
    this->update();
}

void Label::paintEvent(QPaintEvent*)
{
    if (this->text().isEmpty())
        return;

    m_painter = new QPainter(this);
    m_painter->setFont(this->font());

    QPen preEditPen(palette().color(QPalette::Active, QPalette::Highlight));
    QPen frontPen(palette().color(QPalette::Active, QPalette::Text));
    QPen backPen(palette().color(QPalette::Disabled, QPalette::Text));

    int lFront = this->text().length();
    int lBack = this->m_completionText.length();
    m_position = this->contentsRect().width()/2;
    int length = QFontMetrics(this->font()).width(this->text());
    bool drawComletion = false;
    int offset = this->m_completionText.indexOf(this->text(), 0, Qt::CaseInsensitive);
    m_position -= length/2;
    int advance;
    if (length >= this->contentsRect().width()-60)
        m_position = -(length-this->width()+30);
    if (offset >= 0)
    {
        drawComletion = true;
        m_position -= QFontMetrics(this->font()).width(this->m_completionText.left(offset));
    }
    else
    {
        offset = 0;
    }
    backPen = makeGradient(backPen);
    frontPen = makeGradient(frontPen);
    for (int index = 0; index < qMax(lFront, lBack); index++)
    {
        QChar ch;
        if (drawComletion)
        {
            ch = this->m_completionText.at(index);
            if (index-offset < 0 or index-offset >= lFront)
                advance = paintText(ch, backPen);
            else
                advance = paintText(ch, frontPen);
        }
        else
        {
            if (index-offset >= lFront)
                break;
            ch = this->text().at(index);
            advance = paintText(ch, frontPen);
        }
        if (!m_preEditText.isEmpty())
        {
            if (index-lFront > 0 and index-lFront < this->m_preEditText.length())
            {
                ch = this->m_preEditText.at(index-lFront);
                paintText(ch, preEditPen);
            }
        }
        m_position += advance;
    }
    delete m_painter;
    m_painter = 0;
}

int Label::paintText(QChar ch, QPen pen)
{
    int width = QFontMetrics(this->font()).width(ch);
    if (m_position+width >= 0 and m_position <= this->contentsRect().width())
    {
        m_painter->setPen(pen);
        QRect rect = this->contentsRect();
        rect.setLeft(m_position);
        m_painter->drawText(rect, QString(ch));
    }
    return width;
}

QPen Label::makeGradient(QPen pen)
{
    QColor color = pen.color();
    color.setAlpha(0);
    QGradient gradient;
    gradient = QRadialGradient(this->contentsRect().center(), this->contentsRect().width()/2);
    gradient.setColorAt(1, color);
    gradient.setColorAt(0.8, pen.color());
    pen.setBrush(QBrush(gradient));
    return pen;
}


namespace
{
int max(int i1, int i2)
{
    if (i1 > i2)
        return i1;
    else
        return i2;
}

QString longest(QString str1, QString str2)
{
    if (str1.length() > str2.length())
        return str1;
    else
        return str2;
}
};


#include "Label.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
