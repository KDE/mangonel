#ifndef Mangonel_H
#define Mangonel_H

#include "Label.h"
#include "Provider.h"

#include <QGraphicsView>
#include <QLabel>
#include <KDE/Plasma/Dialog>
#include <KDE/KAction>
#include <KDE/KApplication>


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
    enum direction {left, right};
    IconView(QWidget* parent = 0);
    ~IconView();
    void addProgram(Application application);
    Application* selectedApp();
    void moveItems(IconView::direction direction);
    void clear();
    void setFirst();
    Label* label;
private:
    QList<ProgramView*> items;
    QGraphicsScene* scene;
    int current;
};

class AppList : public QList<Application>
{
public:
    AppList();
    ~AppList();
    void insertSorted(Application value);
};

class Mangonel : public Plasma::Dialog
{
    Q_OBJECT
public:
    Mangonel(KApplication* app);
    ~Mangonel();
public slots:
    void show();
    void hide();
private:
    KAction* actionShow;
    bool processingKey;
    QApplication* app;
    Label* label;
    IconView* iconView;
    int historyIndex;
    QStringList history;
    QHash<QString, Provider*> providers;
    AppList* apps;
    int current;
    bool event(QEvent* event);
    void inputMethodEvent(QInputMethodEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    bool eventFilter(QObject *object, QEvent *event);
private slots:
    void launch();
    void getApp(QString query);
    void showHide();
    void showConfig();
    void setHotkey(const QKeySequence& hotkey);
};

#endif // Mangonel_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
