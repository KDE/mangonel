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
    void configureNotifications();
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
