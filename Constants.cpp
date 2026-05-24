#include"Constants.h"
#include<QStandardPaths>
#include<QDir>

namespace JASMINE_CONSTANTS {

const char* APP_VERSION = "1.3.2";
const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Jasmine";
const QString downloadsDirPath = appDirPath + "/Downloads";
const QString screenshotsDirPath = appDirPath + "/Screenshots";
const QString iconDir = appDirPath + "/radio_icons";
const QString iptvDir = appDirPath + "/iptv";

}

