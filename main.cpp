#include "Mangonel.h"

#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>


int main(int argc, char** argv)
{
    KCmdLineArgs::init(argc, argv,
                       QByteArray("Mangonel"),
                       QByteArray("Mangonel"),
                       ki18n("Mangonel"),
                       QByteArray("0.1"),
                       ki18n("A simple application launcher for KDE4."));
    KApplication app;
    app.setOrganizationName("Tarmack SW");
    Mangonel foo(&app);
    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
