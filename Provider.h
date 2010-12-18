#ifndef Provider_H
#define Provider_H

#include <QObject>
#include <QVariant>


class AppList;

// Abstract base class of providers.
class Provider : public QObject
{
public:
    virtual ~Provider() {};
public slots:
    virtual AppList getResults(QString query) = 0;
    virtual void launch(QVariant selected) = 0;
};

// Struct stored in AppList.
struct Application
{
    QString name;
    QString completion;
    QString icon;
    int priority;
    QVariant program;
    Provider* object; //Pointer to the search provider that provided this result.
};

// Search providers return an AppList with their results, they do not need to sort the results.
class AppList : public QList<Application>
{
public:
    AppList();
    ~AppList();
    void insertSorted(Application value);
};

#endif // Provider_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
