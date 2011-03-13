#ifndef Units_H
#define Units_H

#include "Provider.h"


class Units : public Provider
{
    Q_OBJECT
public:
    Units();
    ~Units();
public slots:
    QList<Application> getResults(QString query);
    int launch(QVariant selected);
};
    
    

#endif // Units_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 