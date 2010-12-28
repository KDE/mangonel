#ifndef Mangonel_H
#define Mangonel_H

#include "Label.h"
#include "Provider.h"

#include <QApplication>
#include <QGraphicsView>
#include <QLabel>
#include <KDE/Plasma/Dialog>
#include <KDE/KAction>


class ProgramView : public QGraphicsItemGroup
{
public:
    ProgramView(Application application);
    virtual ~ProgramView();
    void show();
    void shearLeft();
    void shearRight();
    void center();
    void centerItems();
    Application application;
private:
    QGraphicsPixmapItem* icon;
    QGraphicsTextItem* label;
    QGraphicsRectItem* block;
};

class IconView : public QGraphicsView
{
    Q_OBJECT
public:
    IconView(QWidget* parent = 0);
    virtual ~IconView();
    void addProgram(Application application);
    Application* selectedApp();
    void moveItems(QString direction);
    void clear();
    void setFirst();
    Label* label;
private:
    QList<ProgramView*> items;
    QGraphicsScene* scene;
    int current;
};

class Mangonel : public Plasma::Dialog
{
    Q_OBJECT
public:
    Mangonel(QApplication* app);
    virtual ~Mangonel();
    void launch();
    void getApp(QString query);
private:
    KAction* actionShow;
    bool processingKey;
    QApplication* app;
    Label* label;
    IconView* iconView;
    QHash<QString, Provider*> providers;
    AppList* apps;
    int current;
    void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
private slots:
    void showHide(bool type = false);
    void showConfig();
    void setHotkey(const QKeySequence& hotkey);
};

#endif // Mangonel_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
