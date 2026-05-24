#include "mainwindow.h"

#include <QApplication>
#include<QCoreApplication>
#include"Constants.h"
#include<QDir>




int main(int argc, char *argv[])
{

#ifdef FLATPAK_BUILD
    QCoreApplication::setApplicationName("Jasmine");
    QCoreApplication::setOrganizationName("");
#else
    QCoreApplication::setApplicationName("Jasmine-local");
    QCoreApplication::setOrganizationName("Jasmine-local");
#endif

    QCoreApplication::setApplicationVersion(JASMINE_CONSTANTS::APP_VERSION);

    QDir().mkpath(JASMINE_CONSTANTS::appDirPath);
    QDir().mkpath(JASMINE_CONSTANTS::downloadsDirPath);
    QDir().mkpath(JASMINE_CONSTANTS::screenshotsDirPath);
    QDir().mkpath(JASMINE_CONSTANTS::iconDir);
    QDir().mkpath(JASMINE_CONSTANTS::iptvDir);

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
