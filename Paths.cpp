#include "Paths.h"

#include <QDir>
#include <QDBusInterface>
#include <QSettings>
#include <KDE/KIcon>


Paths::Paths()
{}

Paths::~Paths()
{}

AppList Paths::getResults(QString query)
{
    AppList list = AppList();
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
            result.completion += "/";
        result.icon = "system-file-manager";
        result.priority = priority;
        result.object = this;
        result.program = path.absoluteFilePath();
        priority ++;
        list.append(result);
    }
    return list;
}

int Paths::launch(QVariant selected)
{
    // Connect to the KLauncher DBus inteface.
    QDBusInterface* dbus = new QDBusInterface(
        "org.kde.klauncher",
        "/KLauncher",
        "org.kde.KLauncher"
    );
    QString exec = getDefaultApp(selected.toString());
    exec.remove(".desktop");
    exec.remove("\n");
    QDBusMessage msg = dbus->call(
                           "start_service_by_desktop_name",
                           exec,
                           selected.toStringList(),
                           QStringList(),
                           "",
                           true
                       );
    return 0;
}


namespace
{
QString getDefaultApp(QString location)
{
    QString cmd = "xdg-mime";
    QString getMime = " query filetype " + escapePath(location);
    QString mime = getShellCmd(cmd + getMime);
    QString getApp = " query default " + mime;
    QString data = getShellCmd(cmd + getApp);
    return data;
}

QString getShellCmd(QString cmd)
{
    QString data = "";
    FILE* stream;
    int MAX_BUFFER = 256;
    char buffer[MAX_BUFFER];
    cmd.append(" 2>&1");
    stream = popen(cmd.toStdString().c_str(), "r");
    if (stream)
    {
        while (!feof(stream))
        {
            if (fgets(buffer, MAX_BUFFER, stream) != NULL)
            {
                data.append(buffer);
            }
        }
        pclose(stream);
    }
    return data;
}

QString escapePath(QString path)
{
    path.replace(" ", "\\ ");
    return path;
}

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
