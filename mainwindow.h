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
#include"urlbar.h"
#include"RequestInterceptor.h"
#include "simpleadblocker.h"
#include<QWebEngineFullScreenRequest>
#include<QShortcut>
#include "mywebpage.h"
#include"adfreeplayerdialog.h"
#include"radiostation.h"
#include"searchradiostationsdialog.h"
#include<QSystemTrayIcon>
#include"iptvchannel.h"
#include"searchiptvdialog.h"
#include"podcast.h"
#include"searchpodcastdialog.h"
#include"historymanager.h"
#include"bookmarkmanager.h"
#include<QToolButton>

struct SessionData {
    QString name;
    QDateTime timestamp;
    QStringList openTabUrls;
    QStringList openTabTitles;
    QList<QIcon> openTabIcons;
    QIcon icon;  // Session icon
    QString comments;
    bool usingSeparateProfiles;
    QList<bool> tabHasSeparateProfile;  // Which tabs actually have separate profiles
    QStringList tabOriginalProfileNames;  // Original profile names for each tab

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

    QStackedWidget* detailsStack = nullptr;
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

    QAction* m_addAction;
    QAction* m_editAction;
    QAction* m_deleteAction;
    QAction* m_launchAction;
    QAction* m_webBackAction;
    QAction* m_webForwardAction;
    QAction* m_webReloadAction;
    QTabWidget* m_leftPanelTabs;
    QScrollArea* m_websitesScrollArea;
    QScrollArea* m_sessionsScrollArea;
    QWidget* m_websitesContainer;
    QWidget* m_sessionsContainer;
    QGridLayout* m_websitesGrid;
    QGridLayout* m_sessionsGrid;
    QFrame* m_detailsPanel;

    QMap<int, QFrame*> m_websiteCards;
    QMap<QString, QFrame*> m_sessionCards;

    WebsiteListModel* m_model;
    QWebEngineProfile* m_webProfile;
    QMap<QWebEngineView*, QWebEngineProfile*> m_tabProfiles;
    QMap<QString, SessionData> m_sessions;
    bool m_usingSeparateProfiles = false;

    QWidget* createModernDashboard();
    QWidget* createWebViewContainer();
    QFrame* createWebsiteCard(const Website& website, int index);
    QFrame* createSessionCard(const QString& sessionName, const SessionData& session);
    QFrame* createDetailPanel();
    void setupModernStyles();

    void updateWebsiteCards();
    void updateSessionCards();
    void highlightCard(QFrame* card, bool highlight);
    void updateReturnToWebViewButton();

    void showDashboard();
    void showWebViews();

    void populateFormFromWebsite(const Website &website);

    void saveModel();
    WebsiteListModel* loadModel();
    void updateWebsiteList();

    void createMenus();
    QStringList getAvailableSessions();
    void deleteSession(const QString& name);
    void saveSessionsData();
    void loadSessionsData();
    void saveSession(const QString& name);
    void loadSession(const QString& name);

    void setupProfilesCheckbox();
    QWebEngineProfile* createProfileForTab(int tabIndex = -1, const QString& sessionName = QString());
    void cleanupTabProfile(QWebEngineView* webView);

    void ensureProfileDirectoriesExist();
    bool copyProfileData(const QString& sourceDir, const QString& destDir);
    void cleanupSessionProfileDirectories(const QString& sessionName);
    void prepareForShutdown();
    void listDirectoryContents(const QString& dirPath, int level = 0);

    bool hasOpenTabs() const;
    QString getWebsitesFilePath();
    void updateWebsiteIcon(int websiteIndex, const QIcon &icon);
    QLabel* m_iconLabel;


    QFrame* createSessionDetailPanel();
    void populateSessionForm(const SessionData &session);

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
    QLineEdit* m_usernameInput;
    QLineEdit* m_passwordInput;
    QPushButton* m_usernameEyeButton;
    QPushButton* m_passwordEyeButton;
    SecurityManager* m_securityManager;
public:
    bool checkStartupSecurity();
private:
    QPushButton* launchBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QIcon m_originalDownloadIcon;
    bool m_isDarkTheme = false;
    URLBar* m_urlBar;
    QAction* m_toggleUrlBarAction;
    bool isUrlBarVisible = false;
    void updateUrlBarState();
    QSize m_savedWebViewSize;
#ifndef FLATPAK_BUILD
    static const int DASHBOARD_WIDTH = 1220; // orig 1150
    static const int DASHBOARD_HEIGHT = 850; // orig 800
#else
    static const int DASHBOARD_WIDTH = 1230; // orig 1150
    static const int DASHBOARD_HEIGHT = 860; // orig 800
#endif
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
    SimpleAdBlocker *m_adBlocker;

    void injectAdBlockScript(MyWebPage *page);

private:
    bool m_usingNamedProfiles = false;
    QString m_selectedProfileName;
    QMap<QString, QWebEngineProfile*> m_namedProfiles;
    QMap<QWebEngineView*, QString> m_tabNamedProfiles;
    QComboBox* profileSelector = nullptr;
    QWebEngineProfile* getOrCreateNamedProfile(const QString& name);
    void saveNamedProfilesData();
    void loadNamedProfilesData();
    void setupNamedProfilesUI();
    void showProfileManager();
    QToolBar *toolbar = nullptr;
    QStringList profileNames;


public slots:
    void handleFullScreenRequest(QWebEngineFullScreenRequest request);
private:
    int m_lastTabIndex = -1;

    QWebEngineView* m_theaterWebView = nullptr;
    QWebEngineView* m_normalWebView = nullptr;

    bool m_isInTheaterMode = false;

private slots:
    void enterTheaterMode(QWebEngineView* webView);
    void exitTheaterMode(QWebEngineView* webView);

private:
    void configureProfile(QWebEngineProfile* profile);
    void configurePage(MyWebPage* page);
    QAction* m_openCopiedLinkAction;

    MyWebPage *page = nullptr;
    void setupCustomContextMenu(QWebEngineView *view, QWebEngineProfile *profile);
    void setupVideoAdBlocking(QWebEngineProfile *profile);

public slots:
    void onWebViewContextMenu(const QPoint &pos, QWebEngineProfile *profile);
    void createNewTabWithUrlFromLink(const QString &url, QWebEngineView *originView);
private:
    QWebEngineView *devToolsView = nullptr;
    void highlightButtons(bool highlighted);
    bool buttonsHighlighted  = false;
    QToolButton* toggleButton;
    QAction* highlightButtonsAction;
    AdFreePlayerDialog* player = nullptr;
    QPushButton* streamButton;


    private:
        void createRadioTab();
        void updateRadioGrid();  // Renamed from populateRadioGrid (matches website/session pattern)
        void saveRadioStations();
        void loadRadioStations();

        QScrollArea* m_radioScrollArea;
        QWidget* m_radioContainer;
        QGridLayout* m_radioGrid;
        QMap<int, QFrame*> m_radioCards;  // For card highlighting


        QVector<RadioStation> m_radioStations;
        int m_currentRadioIndex;

        QLabel* m_radioIconLabel;
        QLineEdit* m_radioNameEdit;
        QLineEdit* m_radioStreamUrlEdit;
        QLineEdit* m_radioCountryEdit;
        QLineEdit* m_radioGenreEdit;
        QLineEdit* m_radioBitrateEdit;
        QLineEdit* m_radioCodecEdit;
        QTextEdit* m_radioCommentsEdit;

        QPushButton* m_radioPlayButton;
        QPushButton* m_radioStopButton;
        QPushButton* m_radioUpdateButton;
        QPushButton* m_radioDeleteButton;
        QPushButton* m_radioClearButton;

        QFrame* createRadioDetailPanel();
        QFrame* createRadioCard(const RadioStation& station, int index);
        QString getRadioStationsFilePath();
    private slots:
        void onRadioStationSelected(int index);
        void onRadioPlayClicked();
        void onRadioStopClicked();
        void onRadioUpdateClicked();
        void onRadioDeleteClicked();
        void onRadioClearClicked();
        void onRadioAddClicked();
        void onAddRadioStationFromDialog(const RadioStation &station);
        void searchCards(const QString& text);

private:
    SearchRadioStationsDialog* radioSearchDialog = nullptr;
    QPushButton* m_radioGenerateIconButton;
    void showNotification(const QString& msg, int duration);
    QSystemTrayIcon* m_trayIcon;
    QLineEdit* m_searchLineEdit;
    QAction* searchAction;

private:

    void createIPTVTab();
    void updateIPTVGrid();
    void saveIPTVChannels();
    void loadIPTVChannels();


    QFrame* createIPTVCard(const IPTVChannel& channel, int index);
    QFrame* createIPTVDetailPanel();

    QVector<IPTVChannel> m_iptvChannels;
    QMap<int, QFrame*> m_iptvCards;
    int m_currentIPTVIndex;

    QScrollArea* m_iptvScrollArea;
    QWidget* m_iptvContainer;
    QGridLayout* m_iptvGrid;
    QLabel* m_iptvIconLabel;
    QLineEdit* m_iptvNameEdit;
    QLineEdit* m_iptvStreamUrlEdit;
    QLineEdit* m_iptvCategoryEdit;
    QLineEdit* m_iptvCountryEdit;
    QTextEdit* m_iptvCommentsEdit;
    QPushButton* m_iptvPlayButton;
    QPushButton* m_iptvStopButton;
    QPushButton* m_iptvAddButton;
    QPushButton* m_iptvUpdateButton;
    QPushButton* m_iptvDeleteButton;
    QPushButton* m_iptvClearButton;
    QPushButton* m_iptvGenerateIconButton;
    QString getIPTVChannelsFilePath();

private slots:
    void onIPTVStationSelected(int index);
    void onIPTVPlayClicked();
    void onIPTVStopClicked();
    void onIPTVAddClicked();
    void onIPTVUpdateClicked();
    void onIPTVDeleteClicked();
    void onIPTVClearClicked();
    void onAddIPTVChannelsFromDialog(const QVector<IPTVChannel> &channels);

private:
    SearchIPTVDialog* searchIPTVDialog = nullptr;
    void relayoutCards(QGridLayout* grid, const QMap<int, QFrame*>& cards, const QVector<bool>& visible);
    void downloadIPTVIconIfNeeded(int index);
    void sortAllCards();

private:
    void createPodcastTab();
    void updatePodcastGrid();
    void savePodcasts();
    void loadPodcasts();

    QFrame* createPodcastCard(const PodcastShow& show, int index);
    QFrame* createPodcastDetailPanel();

    QVector<PodcastShow> m_podcastShows;
    QMap<int, QFrame*> m_podcastCards;
    int m_currentPodcastIndex;
    int m_currentEpisodeIndex;

    QScrollArea* m_podcastScrollArea;
    QWidget* m_podcastContainer;
    QGridLayout* m_podcastGrid;
    QLabel* m_podcastArtworkLabel;
    QLabel* m_podcastTitleLabel;
    QLabel* m_podcastAuthorLabel;
    QLabel* m_podcastCategoryLabel;
    QLabel* m_podcastDescriptionLabel;
    QListWidget* m_podcastEpisodeList;
    QPushButton* m_podcastRefreshButton;
    QPushButton* m_podcastUnsubscribeButton;
    QPushButton* m_podcastPlayButton;
    QPushButton* m_podcastStopButton;
    void downloadPodcastArtwork(int index);
    void parseRSSFeed(const QByteArray &data, int showIndex);

    QLabel* m_podcastFeedUrlLabel;
    QLabel* m_podcastWebsiteLabel;
    QLabel* m_podcastEpisodeCountLabel;
    QLabel* m_podcastLastUpdatedLabel;
    QTextEdit* m_podcastCommentsEdit;

    SearchPodcastDialog* searchPodcastdialog = nullptr;
private slots:
    void onPodcastShowSelected(int index);
    void onPodcastRefreshClicked();
    void onPodcastUnsubscribeClicked();
    void onPodcastPlayEpisodeClicked(int episodeIndex);
    void onPodcastStopClicked();
    void onAddPodcastFromDialog(const PodcastShow &show);
    void importPodcastSubscriptions();
    void exportPodcastSubscriptions();
    void refreshAllPodcasts();
private:
    int m_currentPlayingPodcastIndex = -1;
    int m_currentPlayingEpisodeIndex = -1;
    HistoryManager* historyManager = nullptr;
    BookmarkManager* bookmarkManager = nullptr;
    void createMenuButton();
    QToolButton* menuButton;
    bool historyEnabled = false;
    QAction* enableHistoryAction;
    //QMap<QWebEngineView*, QMetaObject::Connection> m_historyConnections;
    bool showUrlBar = false;
    QAction* showUrlBarAction;
    void findInPage(const QString& text);

private slots:
    void onHistoryEnabled(QWebEngineView* view);
};

#endif // MAINWINDOW_H

