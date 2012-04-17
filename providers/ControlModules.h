#ifndef CONTROLMODULES_H
#define CONTROLMODULES_H

#include "Provider.h"

#include <kservicegroup.h>

class ControlModules : public Provider
{
    Q_OBJECT
public:
    ControlModules();
    ~ControlModules();
    
public slots:
    QList<Application> getResults(QString query);
    int launch(QVariant selected);
    
private:
    void storePopularities();
    QHash<QString, popularity> m_popularities;
};

#endif//CONTROLMODULES_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
