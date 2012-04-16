#include "Shell.h"

#include <stdlib.h>
#include <QDBusInterface>
#include <QDir>


Shell::Shell()
{
    this->index = QHash<QString, QString>();
    foreach(QString dir, getPathEnv())
    {
        this->index.unite(walkDir(dir));
    }
}

Shell::~Shell()
{}

QList<Application> Shell::getResults(QString query)
{
    QList<Application> list = QList<Application>();
    foreach(QString key, this->index.keys())
    {
        if (key.startsWith(query.left(query.indexOf(" ")), Qt::CaseInsensitive))
        {
            QString args = query.right(query.length() - query.indexOf(" "));
            if (args == query)
                args = "";
            Application app = Application();
            app.name = key + args;
            app.completion = key;
            app.icon = "system-run";
            app.object = this;
            app.program = this->index[key] + args;

            list.append(app);
        }
    }
    return list;
}

int Shell::launch(QVariant selected)
{
    QStringList args = selected.toString().split(" ", QString::SkipEmptyParts);
    QString exec = args.takeFirst();
    QDBusInterface* dbus = new QDBusInterface(
        "org.kde.klauncher",
        "/KLauncher",
        "org.kde.KLauncher"
    );
    dbus->call(
        "exec_blind",
        exec,
        args
    );
    return 0;
}


namespace
{
QHash<QString, QString> walkDir(QString path)
{
    QHash<QString, QString> binList = QHash<QString, QString>();
    QDir dir = QDir(path);
    QFileInfoList list = dir.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach(QFileInfo file, list)
    {
        if (file.isDir())
        {
            if (file.isSymLink() and file.canonicalFilePath() != path)
                binList.unite(walkDir(file.absoluteFilePath()));
        }
        else
        {
            if (file.isExecutable())
                binList.insert(file.fileName(), file.absoluteFilePath());
        }
    }
    return binList;
}

QStringList getPathEnv()
{
    QString pathEnv = getenv("PATH");
    QStringList pathList = pathEnv.split(":", QString::SkipEmptyParts);
    pathList.append(QDir::homePath() + "/bin");
    return pathList;
}
};


#include "Shell.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
