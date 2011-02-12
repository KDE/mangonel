#ifndef Label_H
#define Label_H

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>


class Label : public QLabel
{
    Q_OBJECT
    enum side {left, right, both};
public:
    Label(QWidget* parent = 0);
    virtual ~Label();
    void setText(QString text);
    void appendText(QString text);
    QString completion();
    void setCompletion(QString string);
    QString preEdit();
    void setPreEdit(QString preEdit);
    virtual void paintEvent(QPaintEvent*);
private:
    QString completionText;
    QString preEditText;
    QPen Frontpen;
    QPen Backpen;
    QPen preEditPen;
    int position;
    QPainter* painter;
    int paintText(QChar ch, QPen pen);
    QPen makeGradient(QPen pen);
signals:
    void textChanged(QString text);
};

namespace
{
int max(int i1, int i2);
QString longest(QString str1, QString str2);
};

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
