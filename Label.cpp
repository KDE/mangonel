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
    m_backPen.setBrush(QBrush());
    m_frontPen.setBrush(QBrush());
    m_preEditPen.setBrush(QBrush());
    m_preEditPen.setColor(Plasma::Theme().color(Plasma::Theme::HighlightColor));
    QColor color = Plasma::Theme().color(Plasma::Theme::TextColor);
    m_frontPen.setColor(color);
    color.setAlpha(100);
    m_backPen.setColor(color);
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
    m_backPen = makeGradient(m_backPen);
    m_frontPen = makeGradient(m_frontPen);
    for (int index = 0; index < max(lFront, lBack); index++)
    {
        QChar ch;
        if (drawComletion)
        {
            ch = this->m_completionText.at(index);
            if (index-offset < 0 or index-offset >= lFront)
                advance = paintText(ch, m_backPen);
            else
                advance = paintText(ch, m_frontPen);
        }
        else
        {
            if (index-offset >= lFront)
                break;
            ch = this->text().at(index);
            advance = paintText(ch, m_frontPen);
        }
        if (!m_preEditText.isEmpty())
        {
            if (index-lFront > 0 and index-lFront < this->m_preEditText.length())
            {
                ch = this->m_preEditText.at(index-lFront);
                paintText(ch, m_preEditPen);
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
