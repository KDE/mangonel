#include "Mangonel.h"

#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QMenu>
#include <KDE/Plasma/Theme>
#include <KDE/KWindowSystem>
#include <QTextDocument>
#include <QClipboard>

#include "Config.h"
//Include the providers.
#include "providers/Applications.h"
#include "providers/Paths.h"
#include "providers/Shell.h"
#include "providers/Calculator.h"
#include "providers/Units.h"
#include "providers/ControlModules.h"

#include <unistd.h>

#define WINDOW_WIDTH 220
#define WINDOW_HEIGHT 200

Mangonel::Mangonel(KApplication* app)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setAttribute(Qt::WA_InputMethodEnabled);
    setAttribute(Qt::WA_MouseTracking, false);
    app = app;
    m_processingKey = false;
    m_apps = 0;
    QVBoxLayout* view = new QVBoxLayout(this);
    setLayout(view);
    view->setContentsMargins(0,10,0,8);
    // Setup the search feedback label.
    m_label = new Label(this);
    // Instantiate the visual feedback field.
    m_iconView = new IconView(this);
    // Add all to our layout.
    view->addWidget(m_iconView);
    view->addWidget(m_label);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    m_label->setMaximumWidth(WINDOW_WIDTH - 20);

    // Setup our global shortcut.
    m_actionShow = new KAction(QString("Show Mangonel"), this);
    m_actionShow->setObjectName(QString("show"));
    KShortcut shortcut = m_actionShow->shortcut();
    shortcut.setPrimary(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Space));
    m_actionShow->setGlobalShortcut(shortcut);
    connect(m_actionShow, SIGNAL(triggered()), this, SLOT(showHide()));

    // TODO: Get the stored history.

    // Instantiate the providers.
    m_providers["applications"] = new Applications();
    m_providers["paths"] = new Paths();
    m_providers["shell"] = new Shell();
    m_providers["Calculator"] = new Calculator();
    m_providers["Units"] = new Units();
    m_providers["ControlModules"] = new ControlModules();

    connect(m_label, SIGNAL(textChanged(QString)), this, SLOT(getApp(QString)));
    
    QAction* actionConfig = new QAction(KIcon("configure"), "Configuration", this);
    addAction(actionConfig);
    connect(actionConfig, SIGNAL(triggered(bool)), this, SLOT(showConfig()));
}

Mangonel::~Mangonel()
    // Store history of session.
{}

bool Mangonel::event(QEvent* event)
{
    event->ignore();
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*> (event);
        if (mouseEvent->button() == Qt::MiddleButton)
        {
            event->accept();
            m_label->appendText(QApplication::clipboard()->text(QClipboard::Selection));
        }
        else if (!geometry().contains(mouseEvent->globalPos()))
        {
            hide();
            event->accept();
        }
    }
    else if (event->type() == QEvent::ContextMenu)
    {
        QContextMenuEvent* menuEvent = static_cast<QContextMenuEvent*> (event);
        if (!geometry().contains(menuEvent->globalPos()))
            event->accept();
    }
    if (!event->isAccepted())
        Plasma::Dialog::event(event);
    return true;
}

void Mangonel::inputMethodEvent(QInputMethodEvent* event)
{
    QString text = m_label->text();
    text.chop(event->preeditString().length());
    text = text.mid(0, text.length()+event->replacementStart());
    text.append(event->commitString());
    if (text == "~/")
        text = "";
    text.append(event->preeditString());
    m_label->setPreEdit(event->preeditString());
    m_label->setText(text);
}
void Mangonel::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    IconView::direction direction = IconView::right;
    Application* CurrentApp;
    if (m_processingKey)
        return;
    m_processingKey = true;
    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        launch();
    case Qt::Key_Escape:
        hide();
        break;
    case Qt::Key_Up:
        m_historyIndex += 2;
    case Qt::Key_Down:
        m_historyIndex -= 1;
        if (m_historyIndex >= 0)
        {
            if (m_historyIndex < m_history.length())
                m_label->setText(m_history[m_historyIndex]);
        }
        else
            m_historyIndex = -1;
        break;
    case Qt::Key_Left:
        direction = IconView::left;
    case Qt::Key_Right:
        m_iconView->moveItems(direction);
        CurrentApp = m_iconView->selectedApp();
        if (CurrentApp != 0)
            m_label->setCompletion(CurrentApp->completion);
        break;
    default:
        if (key == Qt::Key_Tab)
        {
            if (!m_label->completion().isEmpty())
                m_label->setText(m_label->completion());
        }
        else if (key == Qt::Key_Backspace)
        {
            QString text = m_label->text();
            text.chop(1);
            if (text == "~/")
                text = "";
            m_label->setText(text);
        }
        else if (event->matches(QKeySequence::Paste))
        {
            m_label->appendText(QApplication::clipboard()->text());
        }
        else
        {
            m_label->appendText(event->text());
        }
    }
    m_processingKey = false;
}

void Mangonel::getApp(QString query)
{
    m_iconView->clear();
    delete m_apps;
    m_apps = 0;
    if (query.length() > 0)
    {
        m_apps = new AppList();
        m_current = -1;
        foreach(Provider* provider, m_providers)
        {
            QList<Application> list = provider->getResults(query);
            foreach(Application app, list)
                m_apps->insertSorted(app);
        }
        if (!m_apps->isEmpty())
        {
            for (int i = 0; i < m_apps->length(); i++)
            {
                m_iconView->addProgram(m_apps->at(i));
            }
            m_iconView->setFirst();
            Application* CurrentApp = m_iconView->selectedApp();
            if (CurrentApp != 0)
                m_label->setCompletion(CurrentApp->completion);
        }
        else
        {
            m_label->setCompletion("");
        }
    }
}

void Mangonel::launch()
{
    m_history.insert(0, m_label->text());
    Application* app = m_iconView->selectedApp();
    if (app != 0)
        app->object->launch(app->program);
}

void Mangonel::showHide()
{
    if (isVisible())
        hide();
    else
        show();
}

void Mangonel::show()
{
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    m_historyIndex = -1;
    QRect screen = qApp->desktop()->screenGeometry(this);
    int x = (screen.width() - geometry().width()) / 2;
    int y = (screen.height() - geometry().height()) / 2;
    move(x, y);
    QWidget::show();
    KWindowSystem::forceActiveWindow(winId());
    setFocus();
}

void Mangonel::hide()
{
    m_label->setText("");
    m_iconView->clear();
    delete m_apps;
    m_apps = 0;
    QWidget::hide();
}

void Mangonel::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    grabMouse();
}

void Mangonel::focusOutEvent(QFocusEvent* event)
{
    releaseMouse();
    if (event->reason() != Qt::PopupFocusReason)
        hide();
}

bool Mangonel::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    if (event->type() == QEvent::FocusOut)
        return true;
    return false;
}

void Mangonel::showConfig()
{
    KShortcut shortcut = m_actionShow->globalShortcut();
    ConfigDialog* dialog = new ConfigDialog(this);
    dialog->setHotkey(shortcut.primary());
    connect(dialog, SIGNAL(hotkeyChanged(QKeySequence)), this, SLOT(setHotkey(QKeySequence)));
    installEventFilter(this);
    releaseMouse();
    dialog->exec();
    removeEventFilter(this);
    activateWindow();
    setFocus();
}

void Mangonel::setHotkey(const QKeySequence& hotkey)
{
    KShortcut shortcut = KShortcut();
    shortcut.setPrimary(hotkey);
    m_actionShow->setGlobalShortcut(shortcut, KAction::ShortcutTypes(KAction::ActiveShortcut|KAction::DefaultShortcut), KAction::NoAutoloading);
    qDebug() << hotkey.toString();
}


IconView::IconView(QWidget* parent) : current(-1)
{
    Q_UNUSED(parent);
    scene = new QGraphicsScene(QRectF(0, 0, rect().width()*4, rect().height()), this);
    setScene(scene);
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background: transparent; border: none");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    centerOn(QPoint(rect().width()*1.5, 0));
}

IconView::~IconView()
{
    delete scene;
}

void IconView::clear()
{
    scene->clear();
    items.clear();
    current = -1;
}

void IconView::addProgram(Application application)
{
    ProgramView* program = new ProgramView(application);
    items.append(program);
    scene->addItem(program);
}

Application* IconView::selectedApp()
{
    if (current >= 0 and current < items.length())
    {
        return &items[current]->application;
    }
    else return 0;
}

void IconView::setFirst()
{
    if (!items.empty())
        current = 0;
    items[current]->show();
    items[current]->setPos(rect().width() + (rect().width() - 128) / 2, 0);
    centerOn(QPoint(rect().width()*1.5, 0));
}

void IconView::moveItems(IconView::direction direction)
{
    if (current < 0)
        return;
    int offset = rect().width();
    int steps =  10;
    int dx = offset / steps;
    int index = 1;
    if (direction == IconView::right)
    {
        if (current + 1 >= items.length())
            return;
        dx = -dx;
        offset *= 2;
    }
    else
    {
        if (current < 1)
            return;
        offset = 0;
        index = -1;
    }
    ProgramView* itemNew = items[current+index];
    ProgramView* itemOld = items[current];
    itemNew->setPos(offset + (rect().width() - 128) / 2, 0);
    itemNew->show();
    int startposNew = itemNew->pos().x();
    int startPosOld = itemOld->pos().x();
    for (int i = 0; i < steps / 2; i++)
    {
        itemNew->setPos(startposNew + (dx * i), 0);
        QApplication::instance()->processEvents();
        usleep(5000);
    }
    startposNew = itemNew->pos().x();
    startPosOld = itemOld->pos().x();
    for (int i = 0; i < steps / 2; i++)
    {
        itemNew->setPos(startposNew + (dx * i), 0);
        itemOld->setPos(startPosOld + (dx * i), 0);
        QApplication::instance()->processEvents();
        usleep(5000);
    }
    startposNew = itemNew->pos().x();
    startPosOld = itemOld->pos().x();
    for (int i = 0; i < steps / 2; i++)
    {
        itemOld->setPos(startPosOld + (dx * i), 0);
        QApplication::instance()->processEvents();
        usleep(5000);
    }
    itemOld->hide();
    itemNew->setPos(rect().width() + (rect().width() - 128) / 2, 0);
    current += index;
    centerOn(QPoint(rect().width()*1.5, 0));
}


ProgramView::ProgramView(Application application)
{
    hide();
    icon = 0;
    label = 0;
    block = 0;
    m_descriptionLabel = 0;
    application = application;
}

ProgramView::~ProgramView()
{
    delete icon;
    delete label;
    delete block;
    delete m_descriptionLabel;
}

void ProgramView::centerItems()
{
    icon->setPos(0, 0);
    QRectF iconRect = icon->boundingRect();
    QRectF labelRect = label->boundingRect();
    QRectF blockRect = block->boundingRect();
    QRectF descriptionRect = m_descriptionLabel->boundingRect();
    block->setPos(
        qreal(iconRect.width() / 2 - blockRect.width() / 2),
        qreal(iconRect.height() / 2 - blockRect.height() / 2)
    );
    label->setPos(
        qreal(iconRect.width() / 2 - labelRect.width() / 2),
        qreal(iconRect.height() / 2 - labelRect.height() / 2)
    );
    m_descriptionLabel->setPos(
        qreal(iconRect.width() / 2 - descriptionRect.width() / 2),
        qreal(iconRect.height() / 2 - descriptionRect.height() / 2 + labelRect.height())
    );
}

void ProgramView::show()
{
    if (icon == 0)
        icon = new QGraphicsPixmapItem(KIcon(application.icon).pixmap(128), this);
    if (label == 0)
    {
        label = new QGraphicsTextItem(application.name, this);
        if (label->boundingRect().width() > WINDOW_WIDTH - 40)
            label->adjustSize();
        label->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
        QColor color = Plasma::Theme().color(Plasma::Theme::TextColor);
        label->setDefaultTextColor(color);
    }
    if (m_descriptionLabel == 0)
    {
        m_descriptionLabel = new QGraphicsTextItem("(" + application.type + ")", this);
        if (m_descriptionLabel->boundingRect().width() > WINDOW_WIDTH - 40)
            m_descriptionLabel->adjustSize();
        m_descriptionLabel->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
        QColor color = Plasma::Theme().color(Plasma::Theme::TextColor);
        m_descriptionLabel->setDefaultTextColor(color);
    }
    if (block == 0)
    {
        QRectF nameRect = label->boundingRect();
        QRectF descriptionRect = m_descriptionLabel->boundingRect();
        QRectF rect(nameRect.x(), nameRect.y() +10, qMax(nameRect.width(), descriptionRect.width()), nameRect.height() + descriptionRect.height() + 5);
        block = new QGraphicsRectItem(rect, this);
        QBrush brush = QBrush(Qt::SolidPattern);
        QColor color = Plasma::Theme().color(Plasma::Theme::BackgroundColor);
        brush.setColor(color);
        block->setBrush(brush);
        block->setOpacity(0.7);
    }
    label->setZValue(10);
    m_descriptionLabel->setZValue(10);
    centerItems();
    QGraphicsItemGroup::show();
}


AppList::AppList()
{}

AppList::~AppList()
{}

void AppList::insertSorted(Application item)
{
    int index = length() / 2;
    if (length() > 0)
    {
        int span = 1 + length() / 2;
        int priority = item.priority;
        int item = value(index).priority;
        while (!(
                    priority > value(index - 1).priority and
                    priority <= item
                ))
        {
            span -= span / 2;
            if (priority > item)
                index += span;
            else if (priority <= item)
                index -= span;
            if (index < 0)
            {
                index = 0;
                break;
            }
            if (index >= length())
            {
                index = length();
                break;
            }
            item = value(index).priority;
        }
    }
    insert(index, item);
}


#include "Mangonel.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
