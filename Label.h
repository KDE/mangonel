#ifndef Label_H
#define Label_H

#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QPaintEvent>


class Label : public QLabel
{
public:
    Label(QWidget* parent = 0);
    virtual ~Label();
    QString completion();
    void setCompletion(QString string);
    virtual void paintEvent(QPaintEvent*);
private:
    QString completionText;
    QPen Frontpen;
    QPen Backpen;
    int position;
    QPainter* painter;
    void paintText(QChar ch, QPainter* painter);
};

int max(int i1, int i2);
QString longest(QString str1, QString str2);
QPen makeGradient(QPen pen);

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
