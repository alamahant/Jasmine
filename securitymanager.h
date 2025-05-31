#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QPushButton>
#include <QAbstractButton>

class QAction;
class QWidget;

class SecurityManager : public QObject
{
    Q_OBJECT

public:
    explicit SecurityManager(QWidget* parent = nullptr);

    // Menu setup
    void setupSecurityMenu(class QMenuBar* menuBar);

    // Startup check
    bool checkPasswordOnStartup();

    // Settings
    bool isPasswordProtectionEnabled() const;
    void clearSecuritySettings();

private slots:
    void onTogglePasswordProtection(bool enabled);

private:
    bool promptForMasterPassword();
    void setMasterPassword(bool isEdit = false);
    bool validateMasterPassword();
    QString hashPassword(const QString& password);

    QWidget* m_parent;
    QAction* m_requirePasswordAction;
    QAction* m_factoryResetAction;
    QSettings m_settings;

signals:
    void factoryResetRequested();



};

#endif // SECURITYMANAGER_H

