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
    QGraphicsPixmapItem* m_icon;
    QGraphicsTextItem* m_label;
    QGraphicsTextItem* m_descriptionLabel;
    QGraphicsRectItem* m_block;
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
    QList<ProgramView*> m_items;
    QGraphicsScene* m_scene;
    int m_current;
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
private slots:
    void launch();
    void getApp(QString query);
    void showHide();
    void showConfig();
    void setHotkey(const QKeySequence& hotkey);
private:
    bool event(QEvent* event);
    void inputMethodEvent(QInputMethodEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    bool eventFilter(QObject *object, QEvent *event);
    
    KAction* m_actionShow;
    bool m_processingKey;
    Label* m_label;
    IconView* m_iconView;
    int m_historyIndex;
    QStringList m_history;
    QHash<QString, Provider*> m_providers;
    AppList* m_apps;
    int m_current;
};

#endif // Mangonel_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
