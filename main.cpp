#include "Mangonel.h"

#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>


int main(int argc, char** argv)
{
    KAboutData* aboutData = new KAboutData(
                       QByteArray("Mangonel"),
                       QByteArray("Mangonel"),
                       ki18n("Mangonel"),
                       QByteArray("0.1"),
                       ki18n("A simple application launcher for KDE4."));
    aboutData->setHomepage(QByteArray("www.tarmack.eu/mangonel/"));
    aboutData->setBugAddress(QByteArray("bugs.mangonel@tarmack.eu"));
    KCmdLineArgs::init(argc, argv, aboutData);
    KApplication app;
    app.setOrganizationName("Tarmack SW");
    Mangonel foo(&app);
    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
