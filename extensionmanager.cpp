#include "extensionmanager.h"
#include <QWebEngineProfile>
#include <QWebEngineExtensionManager>
#include <QWebEngineExtensionInfo>
#include <QDir>
#include <QFileInfo>
#include <QAction>
#include <QDialog>
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QToolBar>
#include <QSettings>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>

ExtensionManager::ExtensionManager(QWebEngineProfile *profile, QObject *parent)
    : QObject(parent)
    , m_profile(profile)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               QCoreApplication::organizationName(),
                               QCoreApplication::applicationName() + "/extensions", this))
{
    if (!m_profile) {
        qWarning() << "ExtensionManager: No valid profile provided";
        return;
    }

    // Ensure sandbox is disabled to prevent crashes
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");

    m_extManager = m_profile->extensionManager();
    if (!m_extManager) {
        qWarning() << "ExtensionManager: Failed to get extension manager";
        return;
    }

    // Connect to the actual signals from the documentation [citation:1][citation:3]
    //connect(m_extManager, &QWebEngineExtensionManager::extensionsChanged,
      //      this, &ExtensionManager::onExtensionsChanged);
    connect(m_extManager, &QWebEngineExtensionManager::installFinished,
            this, &ExtensionManager::onInstallFinished);
    connect(m_extManager, &QWebEngineExtensionManager::loadFinished,
            this, &ExtensionManager::onLoadFinished);
    connect(m_extManager, &QWebEngineExtensionManager::uninstallFinished,
            this, &ExtensionManager::onUninstallFinished);

    refreshExtensionList();
    m_initialized = true;
}

ExtensionManager::~ExtensionManager()
{
    saveSettings();
}

void ExtensionManager::refreshExtensionList()
{
    if (!m_extManager) return;

    QList<QWebEngineExtensionInfo> extensions = m_extManager->extensions();
    m_extensions.clear();

    for (const QWebEngineExtensionInfo &info : extensions) {
        m_extensions[info.id()] = convertInfo(info);
    }
}

ExtensionData ExtensionManager::convertInfo(const QWebEngineExtensionInfo &info) const
{
    ExtensionData data;
    data.id = info.id();
    data.name = info.name();
    data.description = info.description();
    data.popupUrl = info.actionPopupUrl();  // This is the correct way to get popup URL [citation:2][citation:5]
    data.enabled = info.isEnabled();
    data.loaded = info.isLoaded();
    data.installed = info.isInstalled();
    data.path = info.path();
    data.errorString = info.error();
    return data;
}

QWebEngineExtensionInfo ExtensionManager::findExtensionInfo(const QString &id) const
{
    if (!m_extManager) return QWebEngineExtensionInfo();

    QList<QWebEngineExtensionInfo> extensions = m_extManager->extensions();
    for (const QWebEngineExtensionInfo &info : extensions) {
        if (info.id() == id) {
            return info;
        }
    }
    return QWebEngineExtensionInfo();
}

bool ExtensionManager::loadFromPath(const QString &path)
{
    if (!m_initialized || !m_extManager) {
        logError("loadFromPath", path, "Extension manager not initialized");
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        logError("loadFromPath", path, "Path does not exist");
        return false;
    }

    if (fileInfo.isDir()) {
        return loadFromFolder(path);
    } else if (fileInfo.suffix().toLower() == "zip" || fileInfo.suffix().toLower() == "crx") {
        return loadFromZip(path);
    }

    logError("loadFromPath", path, "Unsupported file type");
    return false;
}

bool ExtensionManager::loadFromZip(const QString &zipPath)
{
    if (!m_initialized || !m_extManager) return false;

    qDebug() << "Installing extension from zip:" << zipPath;
    // installExtension takes QString path [citation:3][citation:9]
    m_extManager->installExtension(zipPath);
    return true;
}

bool ExtensionManager::loadFromFolder(const QString &folderPath)
{
    if (!m_initialized || !m_extManager) return false;

    qDebug() << "Loading extension from folder:" << folderPath;
    // For unpacked extensions, use installExtension (it handles both) [citation:1]
    m_extManager->installExtension(folderPath);
    return true;
}

void ExtensionManager::loadPreinstalled(const QString &directory)
{
    QDir extensionsDir(directory);
    if (!extensionsDir.exists()) {
        qDebug() << "Preinstalled extensions directory not found:" << directory;
        return;
    }

    // Look for .zip, .crx files
    QStringList filters;
    filters << "*.zip" << "*.crx";
    QStringList files = extensionsDir.entryList(filters, QDir::Files);

    // Also look for subdirectories (unpacked extensions)
    QStringList dirs = extensionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &file : files) {
        QString fullPath = extensionsDir.absoluteFilePath(file);
        loadFromZip(fullPath);
    }

    for (const QString &dir : dirs) {
        QString fullPath = extensionsDir.absoluteFilePath(dir);
        // Check if it has manifest.json before loading
        if (QFile::exists(fullPath + "/manifest.json")) {
            loadFromFolder(fullPath);
        }
    }
}

bool ExtensionManager::enableExtension(const QString &id)
{
    if (!m_initialized || !m_extManager) return false;

    QWebEngineExtensionInfo info = findExtensionInfo(id);
    if (info.id().isEmpty()) {  // Note: isValid() exists? Actually check if id() is empty
        logError("enableExtension", id, "Extension not found");
        return false;
    }

    // setExtensionEnabled takes QWebEngineExtensionInfo, not QString [citation:3][citation:9]
    m_extManager->setExtensionEnabled(info, true);
    return true;
}

bool ExtensionManager::disableExtension(const QString &id)
{
    if (!m_initialized || !m_extManager) return false;

    QWebEngineExtensionInfo info = findExtensionInfo(id);
    if (info.id().isEmpty()) {
        logError("disableExtension", id, "Extension not found");
        return false;
    }

    m_extManager->setExtensionEnabled(info, false);
    return true;
}

bool ExtensionManager::removeExtension(const QString &id)
{
    if (!m_initialized || !m_extManager) return false;

    QWebEngineExtensionInfo info = findExtensionInfo(id);
    if (info.id().isEmpty()) {
        logError("removeExtension", id, "Extension not found");
        return false;
    }

    // uninstallExtension takes QWebEngineExtensionInfo [citation:3]
    m_extManager->uninstallExtension(info);
    return true;
}

QList<ExtensionData> ExtensionManager::getInstalledExtensions() const
{
    return m_extensions.values();
}

ExtensionData ExtensionManager::getExtensionInfo(const QString &id) const
{
    return m_extensions.value(id);
}

bool ExtensionManager::isExtensionEnabled(const QString &id) const
{
    return m_extensions.contains(id) && m_extensions[id].enabled;
}

bool ExtensionManager::isExtensionInstalled(const QString &id) const
{
    return m_extensions.contains(id);
}

QList<QAction*> ExtensionManager::createToolbarButtons(QWidget *parent)
{
    QList<QAction*> buttons;

    for (const ExtensionData &ext : m_extensions) {
        if (!ext.enabled) continue;

        // Note: No icon available from API, use a default or name-based
        QAction *action = new QAction(ext.name, parent);  // No icon parameter
        action->setData(ext.id);
        action->setToolTip(QString("%1\n%2").arg(ext.name, ext.description));

        connect(action, &QAction::triggered, this, [this, ext]() {
            showExtensionPopup(ext.id, qobject_cast<QWidget*>(sender()->parent()));
        });

        buttons.append(action);
    }

    return buttons;
}

bool ExtensionManager::showExtensionPopup(const QString &id, QWidget *parent)
{
    if (!m_extensions.contains(id) || !m_extensions[id].enabled) {
        return false;
    }

    QUrl popupUrl = m_extensions[id].popupUrl;
    if (popupUrl.isEmpty()) {
        qWarning() << "No popup URL for extension:" << id;
        return false;
    }

    QDialog *popup = new QDialog(parent);
    popup->setWindowTitle(m_extensions[id].name);
    popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    QWebEngineView *popupView = new QWebEngineView(popup);
    popupView->setUrl(popupUrl);  // Use the URL from actionPopupUrl() [citation:2]
    popupView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout(popup);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(popupView);

    popup->resize(400, 500);
    popup->show();

    return true;
}

void ExtensionManager::saveSettings()
{
    if (!m_settings) return;

    m_settings->beginGroup("Extensions");
    for (auto it = m_extensions.begin(); it != m_extensions.end(); ++it) {
        m_settings->setValue(it.key() + "/enabled", it.value().enabled);
    }
    m_settings->endGroup();
    m_settings->sync();
}

void ExtensionManager::onExtensionsChanged()
{
    refreshExtensionList();
    emit extensionsChanged();
}

void ExtensionManager::onInstallFinished(const QWebEngineExtensionInfo &extension)
{
    ExtensionData data = convertInfo(extension);
    m_extensions[extension.id()] = data;
    emit extensionInstalled(data);

    // Auto-enable based on saved settings
    if (m_settings && m_settings->value("Extensions/" + extension.id() + "/enabled", true).toBool()) {
        enableExtension(extension.id());
    }
}

void ExtensionManager::onLoadFinished(const QWebEngineExtensionInfo &extension)
{
    ExtensionData data = convertInfo(extension);
    m_extensions[extension.id()] = data;
    emit extensionLoadFinished(data);
}

void ExtensionManager::onUninstallFinished(const QWebEngineExtensionInfo &extension)
{
    QString id = extension.id();
    m_extensions.remove(id);
    emit extensionUninstalled(id);
}

void ExtensionManager::logError(const QString &operation, const QString &id, const QString &error)
{
    qWarning() << "ExtensionManager:" << operation << "failed for" << id << ":" << error;
    emit extensionFailed(id, error);
}
