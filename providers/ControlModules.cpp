#include "ControlModules.h"



#include <KServiceTypeTrader>
#include <QDebug>
#include <krun.h>
#include <kconfiggroup.h>

ControlModules::ControlModules()
{
    const KConfigGroup config = KGlobal::config()->group("mangonel_controlmodules");
    
    foreach(const QString &key, config.keyList()) {
        QList<QVariant> values = config.readEntry<QList<QVariant> >(key, QList<QVariant>());
        popularity pop;
        pop.count = values[0].toInt();
        pop.lastUse = values[1].toInt();
        m_popularities.insert(key, pop);
    }
}

ControlModules::~ControlModules()
{
    storePopularities();
}

QList< Application > ControlModules::getResults(QString term)
{
    QList<Application> list;
    QString query = "exist Exec and ( (exist Keywords and '%1' ~subin Keywords) or (exist GenericName and '%1' ~~ GenericName) or (exist Name and '%1' ~~ Name) or ('%1' ~~ Exec) )";
    query = query.arg(term);
    KService::List services = KServiceTypeTrader::self()->query("KCModule", query);
    foreach(const KService::Ptr &service, services) {
        if (service->noDisplay())
            continue;
        
        Application app;
        app.name = service->name();
        app.completion = app.name;
        app.icon = service->icon();
        app.object = this;
        app.program = service->exec();
        app.type = i18n("Open control module");
        
        
        // Same calculation as app
        if (m_popularities.contains(service->exec())) {
            app.priority = time(NULL) - m_popularities[service->exec()].lastUse;
            app.priority -= 3600 * m_popularities[service->exec()].count;
        } else {
            app.priority = time(NULL);
        }
        
        list.append(app);
    }
    return list;
}

int ControlModules::launch(QVariant selected)
{
    QString exec = selected.toString();
    popularity pop;
    if (m_popularities.contains(exec)) {
        pop = m_popularities[exec];
        pop.lastUse = time(NULL);
        pop.count++;
    } else {
        pop.lastUse = time(NULL);
        pop.count = 0;
    }
    m_popularities[exec] = pop;
    
    
    storePopularities();
    
    if (KRun::run(exec, KUrl::List(), 0))
        return 0;
    else
        return 1;
}

void ControlModules::storePopularities()
{
    KConfigGroup config = KGlobal::config()->group("mangonel_controlmodules");
    
    foreach(const QString &key, m_popularities.keys()) {
        QList<QVariant> values;
        values.append(m_popularities[key].count);
        values.append(m_popularities[key].lastUse);
        config.writeEntry(key, values);
    }
    config.sync();
}


#include "ControlModules.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
