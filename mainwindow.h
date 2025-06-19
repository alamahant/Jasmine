#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QDateTime>
#include <QMap>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include<QLabel>
#include "websitelistmodel.h"
#include<QListWidget>
#include<QSplitter>
#include<QBuffer>
#include<QToolButton>
#include<QtSvg/QSvgRenderer>
#include<QPainter>
#include<QSize>
#include"downloadmanager.h"
#include"twofamanager.h"
#include"securitymanager.h"
#include"helpmenudialog.h"
#include"urlbar.h"
#include"RequestInterceptor.h"
#include "simpleadblocker.h"
#include "AdBlockScript.h"

// Session data structure
struct SessionData {
    QString name;
    QDateTime timestamp;
    QStringList openTabUrls;
    QStringList openTabTitles;
    QList<QIcon> openTabIcons;
    QIcon icon;  // Session icon
    QString comments;
    bool usingSeparateProfiles;
    // New fields to track actual per-tab profile state
    QList<bool> tabHasSeparateProfile;  // Which tabs actually have separate profiles
    QStringList tabOriginalProfileNames;  // Original profile names for each tab

    // Named profiles fields
    bool usingNamedProfiles = false;
    QString selectedProfileName;
    QList<bool> tabHasNamedProfile;
    QStringList tabNamedProfileNames;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;


private slots:
    void onAddWebsite();
    void onEditWebsite();
    void onDeleteWebsite();
    void onLaunchWebsite();
    void onClearForm();
    void onWebsiteSelected(const QModelIndex &index);
    void onWebsiteDoubleClicked(const QModelIndex &index);
    void onTabCloseRequested(int index);
    void onSaveSession();
    void onLoadSession();
    void onManageSessions();
    void onCleanAllData();

private:

    // UI Components
    QStackedWidget* m_stackedWidget;
    QWidget* m_dashboardWidget;
    QWidget* m_webViewContainer;
    QListView* m_websiteList;
    QLineEdit* m_urlInput;
    QLineEdit* m_titleInput;
    QTextEdit* m_commentsInput;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_launchButton;
    QPushButton* m_clearButton;
    QTabWidget* m_tabWidget;
    QSplitter* m_dashboardSplitter;
    QCheckBox* m_separateProfilesCheckbox;

    // Actions
    QAction* m_addAction;
    QAction* m_editAction;
    QAction* m_deleteAction;
    QAction* m_launchAction;
    QAction* m_webBackAction;
    QAction* m_webForwardAction;
    QAction* m_webReloadAction;

    // Modern UI Components
    QTabWidget* m_leftPanelTabs;
    QScrollArea* m_websitesScrollArea;
    QScrollArea* m_sessionsScrollArea;
    QWidget* m_websitesContainer;
    QWidget* m_sessionsContainer;
    QGridLayout* m_websitesGrid;
    QGridLayout* m_sessionsGrid;
    QFrame* m_detailsPanel;

    // Card tracking
    QMap<int, QFrame*> m_websiteCards;
    QMap<QString, QFrame*> m_sessionCards;

    // Data
    WebsiteListModel* m_model;
    QWebEngineProfile* m_webProfile;
    QMap<QWebEngineView*, QWebEngineProfile*> m_tabProfiles;
    QMap<QString, SessionData> m_sessions;
    bool m_usingSeparateProfiles = false;

    // UI Creation Methods
    QWidget* createModernDashboard();
    QWidget* createWebViewContainer();
    QFrame* createWebsiteCard(const Website& website, int index);
    QFrame* createSessionCard(const QString& sessionName, const SessionData& session);
    QFrame* createDetailPanel();
    void setupModernStyles();

    // UI Update Methods
    void updateWebsiteCards();
    void updateSessionCards();
    void highlightCard(QFrame* card, bool highlight);
    void updateReturnToWebViewButton();

    // Navigation Methods
    void showDashboard();
    void showWebViews();

    // Form Methods
    void populateFormFromWebsite(const Website &website);

    // Data Methods
    void saveModel();
    WebsiteListModel* loadModel();
    void updateWebsiteList();

    // Session Methods
    void createMenus();
    QStringList getAvailableSessions();
    void deleteSession(const QString& name);
    void saveSessionsData();
    void loadSessionsData();
    void saveSession(const QString& name);
    void loadSession(const QString& name);

    // Profile Methods
    void setupProfilesCheckbox();
    QWebEngineProfile* createProfileForTab(int tabIndex = -1, const QString& sessionName = QString());
    void cleanupTabProfile(QWebEngineView* webView);
    void ensureProfileDirectoriesExist();
    bool copyProfileData(const QString& sourceDir, const QString& destDir);
    void cleanupSessionProfileDirectories(const QString& sessionName);
    void prepareForShutdown();
    void listDirectoryContents(const QString& dirPath, int level = 0);

    // Helper Methods
    bool hasOpenTabs() const;
    QString getWebsitesFilePath();
    void updateWebsiteIcon(int websiteIndex, const QIcon &icon);
    QLabel* m_iconLabel;


    //session panel
    QFrame* createSessionDetailPanel();
    void populateSessionForm(const SessionData &session);

    // Session detail panel controls
    QLineEdit* m_sessionNameInput;
    QListWidget* m_sessionTabsDisplay;
    QLabel* m_sessionTimestampLabel;
    QCheckBox* m_sessionSeparateProfilesCheckbox;
    QPushButton* m_sessionDeleteButton;
    QPushButton* m_sessionLoadButton;
    QPushButton* m_sessionClearButton;

private slots:
    void onSaveCurrentSession();
    void onDeleteSession();
    void onLoadSelectedSession();
    void onClearSessionForm();
    void onSessionSelected(const QString &sessionName);
    void toggleView();
    void onAddCurrentWebsite();
    void onUpdateSession();
private:
    QToolBar* createToolbar();
    QAction *m_addCurrentSessionAction;
    QAction *m_addCurrentWebsiteAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_copyUrlAction;
    QAction *m_toggleViewAction;
    void updateTabCountStatus();
    QLabel* m_tabCountLabel;

    // Current selections
    int m_currentWebsiteIndex = -1;
    QString m_currentSessionName = "";
    void selectFirstItemIfNoneSelected();
    QIcon generateRandomSvgIcon();
    QLabel* m_sessionIconLabel;
    QTextEdit* m_sessionCommentsEdit;
    QPushButton* m_sessionUpdateButton;
    QPushButton* m_sessionGenerateIconButton;
    DownloadManager* downloadManager;
    QAction* m_downloadsAction;
    QAction *m_closeAllTabsAction;

    QPushButton *m_separateProfilesToggle;
    void takeScreenshot();
    QAction *m_screenshotAction;
    QAction *m_openDownloadsFolderAction;
    void openDownloadsFolder();
    //theme
    QPushButton *m_themeToggle;
    QString loadDarkTheme();
    QString loadLightTheme();
    QIcon createRotatedIcon(const QString& iconPath, int degrees = 90);
private slots:
    void onCleanSharedProfileData();
    void onRestoreFactoryDefaults();
    void on_Open2faManager();
private:
    void performFactoryReset();
    TwoFAManager* m_twoFAManager;
    QAction* m_open2faManagerAction;
    //username-passwd
    QLineEdit* m_usernameInput;
    QLineEdit* m_passwordInput;
    QPushButton* m_usernameEyeButton;
    QPushButton* m_passwordEyeButton;
    //securitymanager
    SecurityManager* m_securityManager;
public:
    bool checkStartupSecurity();
private:
    QPushButton* launchBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QIcon m_originalDownloadIcon;
    bool m_isDarkTheme = false;
    //urlbar
    URLBar* m_urlBar;
    QAction* m_toggleUrlBarAction;
    bool isUrlBarVisible = false;
    void updateUrlBarState();
    QSize m_savedWebViewSize;
    static const int DASHBOARD_WIDTH = 1150;
    static const int DASHBOARD_HEIGHT = 800;
    void connectUrlBar();
    void createNewTabWithUrl(const QString &url);
    QAction* m_addWebsiteFromUrlAction;
    QAction* m_goHomeAction;
    void createNewTab();
    QWebEngineView* getCurrentWebView() const;
    void cleanupTempProfiles();
    bool hasSymlinksPointingTo(const QString& targetPath);
    RequestInterceptor *interceptor = nullptr;
    void configureBrowserSettings(QWebEngineProfile* profile);
    // AdBlocker
    SimpleAdBlocker *m_adBlocker;

    //void setupAdBlocker(QWebEngineProfile* profile);
    void injectAdBlockScript(QWebEnginePage *page);

private:
    // Named profiles system
    bool m_usingNamedProfiles = false;
    QString m_selectedProfileName;
    QMap<QString, QWebEngineProfile*> m_namedProfiles;
    QMap<QWebEngineView*, QString> m_tabNamedProfiles;
    QComboBox* profileSelector = nullptr;
    // Named profiles methods
    QWebEngineProfile* getOrCreateNamedProfile(const QString& name);
    void saveNamedProfilesData();
    void loadNamedProfilesData();
    void setupNamedProfilesUI();
    void showProfileManager();
    QToolBar *toolbar = nullptr;
    QStringList profileNames;
};

#endif // MAINWINDOW_H

