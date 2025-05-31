#include "mainwindow.h"

#include <QApplication>
#include<QCoreApplication>
#include<QLoggingCategory>
//#include"securitymanager.h"

int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules("*=false");
    QLoggingCategory::setFilterRules("qt.webengine*=false");



#ifdef FLATPAK_BUILD
    // Set QtWebEngine paths for Flatpak
    qputenv("QTWEBENGINEPROCESS_PATH", "/app/libexec/QtWebEngineProcess");
    qputenv("QTWEBENGINE_RESOURCES_PATH", "/app/share/qtwebengine");
    qputenv("QTWEBENGINE_DICTIONARIES_PATH", "/app/share/qtwebengine");
    qputenv("QTWEBENGINE_LOCALES_PATH", "/app/share/qtwebengine/locales");
#endif
    QApplication a(argc, argv);
#ifdef FLATHUB_BUILD
    QCoreApplication::setApplicationName("Jasmine");
    QCoreApplication::setOrganizationName("");
#else
    QCoreApplication::setApplicationName("Jasmine-local");
    QCoreApplication::setOrganizationName("");
#endif
    MainWindow w;
    // Check password protection before showing window
    if (!w.checkStartupSecurity()) {
        return 0;
    }

    w.show();
    return a.exec();
}
