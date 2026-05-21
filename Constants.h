#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include<QStandardPaths>
#include<QDir>

namespace JASMINE_CONSTANTS {

extern const char* APP_VERSION;
extern const QString appDirPath;
extern const QString downloadsDirPath;
extern const QString screenshotsDirPath;
extern const QString iconDir;
extern const QString iptvDir;

extern const QMap<QString, QString> COUNTRY_MAP;
QString getCountryNameFromCode(const QString &code);
QStringList getAllCountryCodes();
}


#endif // CONSTANTS_H
