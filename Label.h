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
