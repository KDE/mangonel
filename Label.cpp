#include "Label.h"

#include <QPen>
#include <qpaintengine.h>
#include <KDE/Plasma/Theme>


Label::Label(QWidget* parent)
{
    QFont* font = new QFont();
    font->setPointSize(font->pointSize()+6);
    this->setFont(*font);
    delete font;
}

Label::~Label()
{}

QString Label::completion()
{
    return this->completionText;
}

void Label::setCompletion(QString string)
{
    this->completionText = string;
    this->update();
}

void Label::paintEvent(QPaintEvent*)
{
    if (this->text().isEmpty())
        return;
    QPainter* painter = new QPainter(this);
    painter->setFont(this->font());
    Backpen.setBrush(QBrush());
    Frontpen.setBrush(QBrush());
    QColor color = Plasma::Theme().color(Plasma::Theme::TextColor);
    Frontpen.setColor(color);
    color.setAlpha(100);
    Backpen.setColor(color);
    int lFront = this->text().length();
    int lBack = this->completionText.length();
    position = this->contentsRect().width()/2;
    int length = QFontMetrics(this->font()).width(this->text());
    bool drawComletion = false;
    int offset = this->completionText.indexOf(this->text(), 0, Qt::CaseInsensitive);
    position -= length/2;
    if (offset >= 0)
    {
        drawComletion = true;
        position -= QFontMetrics(this->font()).width(this->completionText.left(offset));
    }
    else
    {
        offset = 0;
    }
    if (length > this->width()-60)
    {
        position = -(length-this->width()+30);
        Backpen = makeGradient(Backpen);
        Frontpen = makeGradient(Frontpen);
    }
    for (int index = 0; index < max(lFront, lBack); index++)
    {
        QChar ch;
        if (drawComletion)
        {
            if (index-offset < 0 or index-offset >= lFront)
            {
                painter->setPen(Backpen);
            }
            else
                painter->setPen(Frontpen);
            ch = this->completionText.at(index);
        }
        else
        {
            if (index-offset >= lFront)
                break;
            ch = this->text().at(index);
            painter->setPen(Frontpen);
        }
        paintText(ch, painter);
    }
    delete painter;
}

void Label::paintText(QChar ch, QPainter* painter)
{
    QRect rect = this->contentsRect();
    rect.setLeft(position);
    painter->drawText(rect, QString(ch));
    position += QFontMetrics(this->font()).width(ch);
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

QPen makeGradient(QPen pen)
{
    QColor color = QColor();
    color.setAlpha(0);
    QGradient gradient = QLinearGradient(QPoint(0, 0), QPoint(40, 0));
    gradient.setColorAt(0, color);
    gradient.setColorAt(1, pen.color());
    pen.setBrush(QBrush(gradient));
    return pen;
}
};


// kate: indent-mode cstyle; space-indent on; indent-width 4; 
