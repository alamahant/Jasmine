#include "mainwindow.h"

#include <QApplication>
#include<QCoreApplication>
#include<QLoggingCategory>
#include <cstdio>
//#include"securitymanager.h"
#include"Constants.h"
#include<QProcess>

const char* APP_VERSION = "1.2.5";


int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules("qt.webengine*=false");
    //QLoggingCategory::setFilterRules("*=false");

    // Redirect stderr to null (Linux/Mac)
    //freopen("/dev/null", "w", stderr);

/*
#ifdef FLATPAK_BUILD
    // Set QtWebEngine paths for Flatpak
    qputenv("QTWEBENGINEPROCESS_PATH", "/app/libexec/QtWebEngineProcess");
    qputenv("QTWEBENGINE_RESOURCES_PATH", "/app/share/qtwebengine");
    qputenv("QTWEBENGINE_DICTIONARIES_PATH", "/app/share/qtwebengine");
    qputenv("QTWEBENGINE_LOCALES_PATH", "/app/share/qtwebengine/locales");
#endif
*/

    /*
    QProcess process;
       process.start("lspci", QStringList() << "-k");
       process.waitForFinished();
       QString output = process.readAllStandardOutput();

       if (output.contains("NVIDIA", Qt::CaseInsensitive)) {
           // Set environment variable for QWebEngine
           qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");
       }
*/

#ifdef FLATPAK_BUILD
    QCoreApplication::setApplicationName("Jasmine");
    QCoreApplication::setOrganizationName("");
#else
    QCoreApplication::setApplicationName("Jasmine-local");
    QCoreApplication::setOrganizationName("Jasmine-local");
#endif

    QSettings settings;
        if (settings.value("gpu/disable_acceleration", false).toBool()) {
            qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");
        }

    QApplication a(argc, argv);



    MainWindow w;
    // Check password protection before showing window
    if (!w.checkStartupSecurity()) {
        return 0;
    }

    w.show();
    return a.exec();
}
