#ifndef HELPMENU_H
#define HELPMENU_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

enum class HelpType {
    About,
    Features,
    Instructions,
    BestPractices,
    WhatsNew,
    Shortcuts,
    Security,
    TwoFA,
    DataManagement,
    DownloadManagement,
    onSitesAndSessions,
    onSecurity,
    onNewStorageSystem,
    onNamedProfiles,
    onChangelog,
    supportUs
};

class HelpMenuDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpMenuDialog(HelpType type, QWidget *parent = nullptr);

private:
    void setupUI();
    void loadContent(HelpType type);
    QString getTitle(HelpType type);

    // HTML content methods for each enum
    QString getAboutContent();
    QString getFeaturesContent();
    QString getInstructionsContent();
    QString getBestPracticesContent();
    QString getWhatsNewContent();
    QString getShortcutsContent();
    QString getSecurityContent();
    QString getTwoFAContent();
    QString getDataManagementContent();
    QString getDownloadManagerContent();
    QString getSitesSessionsContent();
    QString getOnSecurityContent();
    QString getOnNewStorageSystemContent();
    QString getOnNamedProfilesContent();
    QString getChangelogContent();

    QString getSupportusContent();

    QVBoxLayout *m_layout;
    QTextEdit *m_contentArea;
    QPushButton *m_closeButton;
};

#endif // HELPMENU_H

