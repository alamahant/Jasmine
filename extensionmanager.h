#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QIcon>
#include <QPointer>
#include<QUrl>
#include <QWebEngineProfile>
#include <QWebEngineExtensionManager>
#include <QWebEngineExtensionInfo>
#include <QAction>
#include <QWidget>
#include <QSettings>

struct ExtensionData {
    QString id;
    QString name;
    QString description;
    QUrl popupUrl;  // Use actionPopupUrl()
    bool enabled = false;
    bool loaded = false;
    bool installed = false;
    QString path;
    QString errorString;
    // Note: No version or icon - not available in API
};

class ExtensionManager : public QObject
{
    Q_OBJECT

public:
    explicit ExtensionManager(QWebEngineProfile *profile, QObject *parent = nullptr);
    ~ExtensionManager();

    // Core extension operations
    bool loadFromPath(const QString &path);
    bool loadFromZip(const QString &zipPath);
    bool loadFromFolder(const QString &folderPath);
    void loadPreinstalled(const QString &directory);

    // Management - these now need to find the extension by ID first
    bool enableExtension(const QString &id);
    bool disableExtension(const QString &id);
    bool removeExtension(const QString &id);

    // Query
    QList<ExtensionData> getInstalledExtensions() const;
    ExtensionData getExtensionInfo(const QString &id) const;
    bool isExtensionEnabled(const QString &id) const;
    bool isExtensionInstalled(const QString &id) const;

    // UI helpers
    QList<QAction*> createToolbarButtons(QWidget *parent = nullptr);
    bool showExtensionPopup(const QString &id, QWidget *parent = nullptr);
    void saveSettings();

signals:
    void extensionsChanged();
    void extensionInstalled(const ExtensionData &data);
    void extensionLoadFinished(const ExtensionData &data);
    void extensionUninstalled(const QString &id);
    void extensionFailed(const QString &id, const QString &error);

private slots:
    void onExtensionsChanged();
    void onInstallFinished(const QWebEngineExtensionInfo &extension);
    void onLoadFinished(const QWebEngineExtensionInfo &extension);
    void onUninstallFinished(const QWebEngineExtensionInfo &extension);

private:
    void refreshExtensionList();
    QWebEngineExtensionInfo findExtensionInfo(const QString &id) const;
    ExtensionData convertInfo(const QWebEngineExtensionInfo &info) const;
    void logError(const QString &operation, const QString &id, const QString &error);

    QPointer<QWebEngineProfile> m_profile;
    QWebEngineExtensionManager *m_extManager;
    QMap<QString, ExtensionData> m_extensions;
    QScopedPointer<QSettings> m_settings;
    bool m_initialized = false;
};

#endif // EXTENSIONMANAGER_H
