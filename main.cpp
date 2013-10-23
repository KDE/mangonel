/*
 * Copyright 2010-2012 Bart Kroon <bart@tarmack.eu>
 * Copyright 2012, 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Mangonel.h"

#include <KDE/KUniqueApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>


int main(int argc, char** argv)
{
    KAboutData* aboutData = new KAboutData(
                       QByteArray("mangonel"),
                       QByteArray("mangonel"),
                       ki18n("Mangonel"),
                       QByteArray("1.1"),
                       ki18n("A simple application launcher for KDE4."));
    aboutData->setHomepage(QByteArray("www.tarmack.eu/mangonel/"));
    aboutData->addAuthor(ki18n("Martin Sandsmark"), ki18n("Developer"), "martin.sandsmark@kde.org", "http://iskrembilen.com/");
    aboutData->addAuthor(ki18n("Bart Kroon"), ki18n("Developer, original author"), "", "http://tarmack.eu/");
    KCmdLineArgs::init(argc, argv, aboutData);
    KUniqueApplication app;
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("Tarmack SW");
    Mangonel foo(&app);
    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
