#ifndef APPLICATIONS_H
#define APPLICATIONS_H

#include "Provider.h"

#include <kservicegroup.h>

class Applications : public Provider
{
    Q_OBJECT
public:
    Applications();
    ~Applications();
    
public slots:
    QList<Application> getResults(QString query);
    int launch(QVariant selected);
    
private:
    void storePopularities();
    QHash<QString, popularity> m_popularities;
};

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
