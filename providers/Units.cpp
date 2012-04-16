#include "Units.h"

#include "providers/units/units.h"

#include <QRegExp>
#include <QClipboard>
#include <QApplication>

Units::Units()
{}

Units::~Units()
{}

QList< Application > Units::getResults(QString query)
{
    QList<Application> list = QList<Application>();
    QRegExp patern = QRegExp("(.+)\\s+(?:\\=|to|is)\\s+(.+)$", Qt::CaseInsensitive);
    if (query.contains(patern) && patern.captureCount() == 2)
    {
            Application result = Application();
            result.icon = "accessories-calculator";
            result.object = this;
        char* value = new char[patern.cap(1).size() + 1];
        strcpy(value, patern.cap(1).toAscii().data());
        char* target = new char[patern.cap(2).size() + 1];
        strcpy(target, patern.cap(2).toAscii().data());
        units_clear_exception();
        QString awnser = QString::number(units_convert(value, target), 'g', 12);
        delete value;
        delete target;
        if (units_check_exception() == 0)
        {
            result.name = awnser;
            result.program = awnser;
        }
        else
        {
            result.name = QString(units_check_exception());
            result.program = result.name;
        }
            list.append(result);
    }
    return list;
}

int Units::launch(QVariant selected)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(selected.toString(), QClipboard::Selection);
    return 0;
}

#include "Units.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 