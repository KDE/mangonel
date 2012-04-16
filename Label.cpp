#include "Label.h"

#include <KDE/Plasma/Theme>


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
    return this->completionText;
}

void Label::setCompletion(QString string)
{
    this->completionText = string;
    this->update();
}

QString Label::preEdit()
{
    return this->preEditText;
}

void Label::setPreEdit(QString preEdit)
{
    this->preEditText = preEdit;
    this->update();
}

void Label::paintEvent(QPaintEvent*)
{
    if (this->text().isEmpty())
        return;
    painter = new QPainter(this);
    painter->setFont(this->font());
    Backpen.setBrush(QBrush());
    Frontpen.setBrush(QBrush());
    preEditPen.setBrush(QBrush());
    preEditPen.setColor(Plasma::Theme().color(Plasma::Theme::HighlightColor));
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
    int advance;
    if (length >= this->contentsRect().width()-60)
        position = -(length-this->width()+30);
    if (offset >= 0)
    {
        drawComletion = true;
        position -= QFontMetrics(this->font()).width(this->completionText.left(offset));
    }
    else
    {
        offset = 0;
    }
    Backpen = makeGradient(Backpen);
    Frontpen = makeGradient(Frontpen);
    for (int index = 0; index < max(lFront, lBack); index++)
    {
        QChar ch;
        if (drawComletion)
        {
            ch = this->completionText.at(index);
            if (index-offset < 0 or index-offset >= lFront)
                advance = paintText(ch, Backpen);
            else
                advance = paintText(ch, Frontpen);
        }
        else
        {
            if (index-offset >= lFront)
                break;
            ch = this->text().at(index);
            advance = paintText(ch, Frontpen);
        }
        if (!preEditText.isEmpty())
        {
            if (index-lFront > 0 and index-lFront < this->preEditText.length())
            {
                ch = this->preEditText.at(index-lFront);
                paintText(ch, preEditPen);
            }
        }
        position += advance;
    }
    delete painter;
    painter = 0;
}

int Label::paintText(QChar ch, QPen pen)
{
    int width = QFontMetrics(this->font()).width(ch);
    if (position+width >= 0 and position <= this->contentsRect().width())
    {
        painter->setPen(pen);
        QRect rect = this->contentsRect();
        rect.setLeft(position);
        painter->drawText(rect, QString(ch));
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
