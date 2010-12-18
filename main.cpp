#include "Mangonel.h"

#include <QtGui/QApplication>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("Tarmack SW");
    app.setApplicationName("Mangonel");
    Mangonel foo(&app);
    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
