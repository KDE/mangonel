#include "Paths.h"

#include <QDir>
#include <KDE/KFileItem>
#include <klocale.h>


Paths::Paths()
{}

Paths::~Paths()
{}

QList<Application> Paths::getResults(QString query)
{
    QList<Application> list = QList<Application>();
    QString original = query;
    QDir dir;
    if (query.startsWith("/"))
    {
        dir = QDir::root();
        query.remove(0, 1);
    }
    else
        dir = QDir::home();
    QStringList walk = query.split("/", QString::SkipEmptyParts);
    if (walk.isEmpty())
        walk.append("");
    QString part = walk.takeFirst();
    while (walk.length() > 0)
    {
        dir.cd(part);
        part = walk.takeFirst();
    }
    query = part + "*";
    QFileInfoList paths = dir.entryInfoList(QStringList(query), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    int priority = 0;
    foreach(QFileInfo path, paths)
    {
        Application result = Application();
        result.name = subUser(path.absoluteFilePath());
        result.completion = result.name.left(result.name.lastIndexOf("/")) + "/" + path.fileName();
        if (path.isDir())
        {
            result.completion += "/";
            result.icon = "system-file-manager";
        }
        else
        {
            KFileItem info = KFileItem(KFileItem::Unknown, KFileItem::Unknown, path.absoluteFilePath());
            result.icon = info.iconName();
        }
        result.priority = priority;
        result.object = this;
        result.program = path.absoluteFilePath();
        result.type = i18n("Open path");
        priority ++;
        list.append(result);
    }
    return list;
}

int Paths::launch(QVariant selected)
{
    KFileItem info = KFileItem(KFileItem::Unknown, KFileItem::Unknown, selected.toString());
    info.run();
    return 0;
}


namespace
{
QString subUser(QString path)
{
    QString homePath = QDir::homePath();
    if (path.startsWith(homePath))
        path = "~" + path.mid(homePath.length(), -1);
    return path;
}
};


#include "Paths.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
