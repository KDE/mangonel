#ifndef Provider_H
#define Provider_H

#include <QObject>
#include <QVariant>

struct popularity
{
    int lastUse;
    int count;
};

class Application;

// Abstract base class of providers.
class Provider : public QObject
{
public:
    virtual ~Provider() {};
public slots:
    virtual QList<Application> getResults(QString query) = 0;
    virtual int launch(QVariant selected) = 0;
};

// Struct stored in AppList.
struct Application
{
    Application() : priority(2147483647)
    {};

    QString name;
    QString completion;
    QString icon;
    int priority;
    QVariant program;
    Provider* object; //Pointer to the search provider that provided this result.
    QString type;
};

#endif // Provider_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
