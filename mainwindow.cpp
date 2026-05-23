
#include "mainwindow.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>
#include <QIcon>
#include <QStyle>
#include <QFormLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>
#include <QWebEngineCookieStore>
#include <QCoreApplication>
#include <QInputDialog>
#include <QListWidget>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QApplication>
#include<QClipboard>
#include<QWebEngineHistory>
#include<QWebEngineHistoryItem>
#include <QComboBox>
#include<QProgressDialog>
#include<QWebEngineSettings>
#include<QClipboard>
#include<QGuiApplication>
#include<QRegularExpression>
#include<QWebEngineContextMenuRequest>
#include<QWebEngineScript>
#include<QWebEngineScriptCollection>
#include"donationdialog.h"
#include"Constants.h"
#include<QPainter>
#include<QSystemTrayIcon>
#include<QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      page(nullptr),
      devToolsView(new QWebEngineView),
      player(new AdFreePlayerDialog(this)),
      radioSearchDialog(new SearchRadioStationsDialog(this)),
      searchIPTVDialog(new SearchIPTVDialog(this)),
      searchPodcastdialog(new SearchPodcastDialog(this))

{
    setWindowTitle("Jasmine");
    setWindowIcon(QIcon(":/resources/jasmine.png"));
    //this->setFixedSize(1130, 800);
    this->setFixedSize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);


    QSettings settings;
    buttonsHighlighted = settings.value("buttonsHighlighted", true).toBool();

    //resize(1130, 800);
    //this->installEventFilter(this);


    // Load stylesheet
    QFile styleFile(":/resources/stylesheet.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        setStyleSheet(stream.readAll());
        styleFile.close();
    } else {
        setupModernStyles();
    }

    // Create the stacked widget as the central widget
    m_stackedWidget = new QStackedWidget(this);

    setCentralWidget(m_stackedWidget);


    // Create the dashboard and add it to the stacked widget
    m_dashboardWidget = createModernDashboard();
    m_stackedWidget->addWidget(m_dashboardWidget);

    // add urlbar
    m_urlBar = new URLBar(this);

    m_urlBar->setObjectName("urlBar");
    m_urlBar->setVisible(false); // Initially hidden
    // Create the web view container and add it to the stacked widget
    m_webViewContainer = createWebViewContainer();
    m_stackedWidget->addWidget(m_webViewContainer);


    // Load sessions data
    loadSessionsData();

    // Load the model
    if (QFile::exists(getWebsitesFilePath())) {
        m_model = loadModel();
    } else {
        m_model = new WebsiteListModel(this);
    }

    // Set the model to the list view (keep this for compatibility)
    if (m_websiteList) {
        m_websiteList->setModel(m_model);
    }

    // Update the website cards
    updateWebsiteCards();
    updateSessionCards();

    //
    // Select the first website if we're on the websites tab
    if (m_leftPanelTabs->currentIndex() == 0 && m_model->rowCount() > 0) {
        QModelIndex firstIndex = m_model->index(0, 0);
        m_websiteList->setCurrentIndex(firstIndex); // Add this line

        onWebsiteSelected(firstIndex);
    }
    // Select the first session if we're on the sessions tab
    else if (m_leftPanelTabs->currentIndex() == 1 && !m_sessions.isEmpty()) {
        QString firstName = m_sessions.keys().first();
        onSessionSelected(firstName);
    }
    //
    // Setup web profile
    m_webProfile = new QWebEngineProfile("WebsiteManager", this);

    //m_extManager = new ExtensionManager(m_webProfile,this);

    interceptor = new RequestInterceptor(this);
    m_adBlocker = new SimpleAdBlocker(this);
    m_webProfile->setUrlRequestInterceptor(interceptor);
    m_webProfile->setUrlRequestInterceptor(m_adBlocker);

    configureBrowserSettings(m_webProfile);
    m_webProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    m_webProfile->setPersistentStoragePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/storage");
    ensureProfileDirectoriesExist();

    // Create toolbar with actions

    QToolBar* toolbar = createToolbar();


    // Connect list view signals (keep for compatibility)
    connect(m_websiteList, &QListView::clicked, this, &MainWindow::onWebsiteSelected);
    connect(m_websiteList, &QListView::doubleClicked, this, &MainWindow::onWebsiteDoubleClicked);

    // Connect tab widget signals
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    // Set up keyboard shortcuts
    new QShortcut(QKeySequence("Ctrl+A"), this, SLOT(onAddWebsite()));
    new QShortcut(QKeySequence("Ctrl+E"), this, SLOT(onEditWebsite()));
    new QShortcut(QKeySequence("Delete"), this, SLOT(onDeleteWebsite()));
   // new QShortcut(QKeySequence("Ctrl+L"), this, SLOT(ebsite()));
    //new QShortcut(QKeySequence("Escape"), this, SLOT(onBackToDashboard()));

    // Start with the dashboard view
    showDashboard();
    //initialize downloadmanager
    downloadManager = new DownloadManager(this);
    //connect download icon change
    connect(downloadManager, &DownloadManager::activeDownloadsChanged, [this](int count) {
        if (count > 0) {

            m_downloadsAction->setIcon(QIcon(":/resources/icons/download-green.svg"));
        } else {
            if(m_isDarkTheme){
                m_downloadsAction->setIcon(QIcon(":/resources/icons-white/download.svg")); // Restore original

            } else {
                m_downloadsAction->setIcon(QIcon(":/resources/icons/download.svg")); // Restore original

            }
        }
    });
    //
    // Connect downloads to download manager
    connect(m_webProfile, &QWebEngineProfile::downloadRequested,
            downloadManager, &DownloadManager::handleDownloadRequest);

    //security manager
    m_securityManager = new SecurityManager(this);
    connect(m_securityManager, &SecurityManager::factoryResetRequested,
            this, &MainWindow::onRestoreFactoryDefaults);
    //
    createMenus();
    //2FA
    m_twoFAManager = nullptr;
    //connect urlbar
    connectUrlBar();
    //cleanu temp profiles
    cleanupTempProfiles();

    // Load named profiles

    QTimer::singleShot(100, this, [this]() {
        // First load the profile data
        loadNamedProfilesData();

        // Then set up the UI
        setupNamedProfilesUI();
    });

    statusBar()->showMessage("Ready");

    //testing extensions
    //QString extensionPath = "/home/dharma/.local/share/Alamahant/Jasmine-extensions/storage/Extensions/uBOLite_2026.301.2014.chromium_XWsHZP";
       // Or your AdGuard path
    //QString extensionPath = "/home/dharma/shared/downloads/adguard/";
    //QString extensionPath = "/home/dharma/.local/share/Alamahant/Jasmine-extensions/storage/Extensions/addguard_YSc6qX/";
       //m_extManager->loadFromFolder(extensionPath);

    connect(player, &AdFreePlayerDialog::requestCurrentUrl, this, [this]() {
        if(m_urlBar){
        QString currentUrl = m_urlBar->urlInput()->text();
        if (!currentUrl.isEmpty()) {
            showStreamLoadingDialog(7000);
            player->setUrl(currentUrl);
        }
        }
    });

    /*
    connect(player, &AdFreePlayerDialog::mediaLoaded, [this]() {
        // Player started playing
        m_radioPlayButton->setEnabled(false);
        m_radioStopButton->setEnabled(true);
        m_iptvPlayButton->setEnabled(false);
        m_iptvStopButton->setEnabled(true);
    });

    */

    connect(player, &AdFreePlayerDialog::mediaStopped, [this]() {
        // Player stopped
        m_radioPlayButton->setEnabled(true);
        m_radioStopButton->setEnabled(false);
        m_iptvPlayButton->setEnabled(true);
        m_iptvStopButton->setEnabled(false);

        // Clear all playing indicators
        for (QFrame* card : m_radioCards) {
            card->setProperty("playing", false);
            card->style()->unpolish(card);
            card->style()->polish(card);
        }
        for (QFrame* card : m_iptvCards) {
            card->setProperty("playing", false);
            card->style()->unpolish(card);
            card->style()->polish(card);
        }
    });

    connect(player, &AdFreePlayerDialog::dialogClosed, [this]() {
        streamButton->setChecked(false);
    });

    connect(radioSearchDialog, &SearchRadioStationsDialog::stationSelected,
            this, &MainWindow::onAddRadioStationFromDialog);

    connect(radioSearchDialog, &SearchRadioStationsDialog::showNotification, this, [this](int duration){
        showStreamLoadingDialog(duration);
    });
    connect(searchIPTVDialog, &SearchIPTVDialog::channelsSelected,
            this, &MainWindow::onAddIPTVChannelsFromDialog);

    connect(searchIPTVDialog, &SearchIPTVDialog::previewChannel,
            [this](const QString &streamUrl, const QString &name) {
                showStreamLoadingDialog(7000);
                player->setUrl(streamUrl);
                player->setWindowTitle(QString("Preview: %1").arg(name));
                player->play();
                player->show();
            });


    connect(searchPodcastdialog, &SearchPodcastDialog::podcastSelected,
            this, &MainWindow::onAddPodcastFromDialog);

    //sortAllCards(); // maybe dangerous needs further testing

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/resources/jasmine.png"));
    m_trayIcon->setToolTip("Jasmine Browser");
    m_trayIcon->show();

}

MainWindow::~MainWindow() {

    //saveSessionsData();
    // Save model on exit
    saveModel();

    if(m_toggleViewAction)
        delete m_toggleViewAction;


    // Close download window before deleting download manager
    if (downloadManager) {
        delete downloadManager;
        downloadManager = nullptr;
    }

    if(m_trayIcon) delete m_trayIcon;

}

QWidget* MainWindow::createModernDashboard() {
    QWidget* dashboard = new QWidget(this);

    // Main layout
    QVBoxLayout* containerLayout = new QVBoxLayout(dashboard);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);


    // Create main horizontal splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(3); // was 1


    mainSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background-color: #cccccc;
            margin: 1px;
        }
        QSplitter::handle:hover {
            background-color: #999999;
        }
    )");
    mainSplitter->setChildrenCollapsible(true); // Allow collapsing
    containerLayout->addWidget(mainSplitter, 1);

    // Create left panel with tabs
    m_leftPanelTabs = new QTabWidget();
    m_leftPanelTabs->setTabPosition(QTabWidget::West);
    m_leftPanelTabs->setDocumentMode(true);

    // Create websites tab content
    m_websitesScrollArea = new QScrollArea();
    m_websitesScrollArea->setWidgetResizable(true);
    m_websitesScrollArea->setFrameShape(QFrame::NoFrame);
    m_websitesContainer = new QWidget();
    m_websitesGrid = new QGridLayout(m_websitesContainer);

    m_websitesGrid->setSpacing(12);
    m_websitesGrid->setContentsMargins(16, 16, 16, 16);
    //
    m_websitesGrid->setSizeConstraint(QLayout::SetFixedSize);
    for (int i = 0; i < 3; ++i) { // Assuming 3 columns
        m_websitesGrid->setColumnStretch(i, 0);
    }
    m_websitesScrollArea->setWidget(m_websitesContainer);

    // Create sessions tab content
    m_sessionsScrollArea = new QScrollArea();
    m_sessionsScrollArea->setWidgetResizable(true);
    m_sessionsScrollArea->setFrameShape(QFrame::NoFrame);
    m_sessionsContainer = new QWidget();
    m_sessionsGrid = new QGridLayout(m_sessionsContainer);
    m_sessionsGrid->setSpacing(12);
    m_sessionsGrid->setContentsMargins(16, 16, 16, 16);
    //
    m_sessionsGrid->setSizeConstraint(QLayout::SetFixedSize);
    for (int i = 0; i < 3; ++i) { // Assuming 3 columns
        m_sessionsGrid->setColumnStretch(i, 0);
    }
    m_sessionsScrollArea->setWidget(m_sessionsContainer);

    m_leftPanelTabs->addTab(m_websitesScrollArea, createRotatedIcon(":/resources/icons/globe.svg"), QString());
    m_leftPanelTabs->addTab(m_sessionsScrollArea, createRotatedIcon(":/resources/icons/bookmark.svg"), QString());
    // Set icon size
    m_leftPanelTabs->setIconSize(QSize(120, 15));

    // Add tooltips to the tabs
    m_leftPanelTabs->tabBar()->setTabToolTip(0, "Websites");
    m_leftPanelTabs->tabBar()->setTabToolTip(1, "Sessions");

    // Create a stacked widget for the details panels
    detailsStack = new QStackedWidget();

    // Create website details panel
    m_detailsPanel = createDetailPanel();

    // Set minimum size for details panel
    m_detailsPanel->setMinimumWidth(200); // Set a smaller minimum width

    // Adjust size policies for form elements to allow shrinking
    m_urlInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_commentsInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    detailsStack->addWidget(m_detailsPanel);

    // Create session details panel
    QFrame* sessionDetailsPanel = createSessionDetailPanel();
    sessionDetailsPanel->setObjectName("sessionDetailsPanel");

    // Set minimum size for session details panel
    sessionDetailsPanel->setMinimumWidth(200); // Set a smaller minimum width

    // Adjust size policies for session form elements
    m_sessionNameInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_sessionTabsDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    detailsStack->addWidget(sessionDetailsPanel);

    // Add panels to splitter
    mainSplitter->addWidget(m_leftPanelTabs);
    mainSplitter->addWidget(detailsStack);

    // Set initial sizes (adjust as needed)
    mainSplitter->setSizes(QList<int>() << 400 << 300);

    connect(m_leftPanelTabs, &QTabWidget::currentChanged, [this](int index) {
        // Switch the details stack to match the tab
        if (index >= 0 && index < detailsStack->count()) {
            detailsStack->setCurrentIndex(index);
        }

        // Restore selection based on the active tab
        if (index == 0) { // Websites tab
            if (m_currentWebsiteIndex >= 0 && m_currentWebsiteIndex < m_model->rowCount()) {
                QModelIndex modelIndex = m_model->index(m_currentWebsiteIndex, 0);
                onWebsiteSelected(modelIndex);
            } else if (m_model->rowCount() > 0) {
                QModelIndex firstIndex = m_model->index(0, 0);
                onWebsiteSelected(firstIndex);
            }
        } else if (index == 1) { // Sessions tab
            if (!m_currentSessionName.isEmpty() && m_sessions.contains(m_currentSessionName)) {
                onSessionSelected(m_currentSessionName);
            } else if (!m_sessions.isEmpty()) {
                QString firstName = m_sessions.keys().first();
                onSessionSelected(firstName);
            }
        } else if (index == 2) { // Radio tab
            if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {
                onRadioStationSelected(m_currentRadioIndex);
            } else if (!m_radioStations.isEmpty()) {
                onRadioStationSelected(0);
            }
        } else if (index == 3) { // IPTV tab
            if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {
                onIPTVStationSelected(m_currentIPTVIndex);
            } else if (!m_iptvChannels.isEmpty()) {
                onIPTVStationSelected(0);
            }
        } else if (index == 4) { // Podcast tab
            if (m_currentPodcastIndex >= 0 && m_currentPodcastIndex < m_podcastShows.size()) {
                onPodcastShowSelected(m_currentPodcastIndex);
            } else if (!m_podcastShows.isEmpty()) {
                onPodcastShowSelected(0);
            }
        }
    });

    // Create a hidden list view for backward compatibility
    m_websiteList = new QListView();
    m_websiteList->setVisible(false);

    // radio
    createRadioTab();
    loadRadioStations();
    if (!m_radioStations.isEmpty()) {
        onRadioStationSelected(0);
    }
    updateRadioGrid();

    // iptv
    createIPTVTab();
    loadIPTVChannels();
    if (!m_iptvChannels.isEmpty()) {
        onIPTVStationSelected(0);
    }
    updateIPTVGrid();
    // podcast
    createPodcastTab();
    loadPodcasts();
    if (!m_podcastShows.isEmpty()) {
        onPodcastShowSelected(0);
    }
    updatePodcastGrid();

    return dashboard;

}

QFrame* MainWindow::createDetailPanel() {
    QFrame* panel = new QFrame();
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setObjectName("detailsPanel");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);

    // Title
    QLabel* titleLabel = new QLabel("Website Details");
    titleLabel->setObjectName("detailsPanelTitle");
    layout->addWidget(titleLabel);

    // Form fields
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);


    m_urlInput = new QLineEdit();
    m_titleInput = new QLineEdit();

    // Add username and password fields
    m_usernameInput = new QLineEdit();
    m_usernameInput->setEchoMode(QLineEdit::Password);

    m_passwordInput = new QLineEdit();
    m_passwordInput->setEchoMode(QLineEdit::Password);

    // Create eye icon buttons
    m_usernameEyeButton = new QPushButton();
    m_passwordEyeButton = new QPushButton();
    m_usernameEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));
    m_passwordEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));
    m_usernameEyeButton->setFixedSize(24, 24);
    m_passwordEyeButton->setFixedSize(24, 24);
    m_usernameEyeButton->setCheckable(true);
    m_passwordEyeButton->setCheckable(true);
    m_commentsInput = new QTextEdit();
    m_commentsInput->setMinimumHeight(100);

    // Icon display
    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFrameShape(QFrame::StyledPanel);
    m_iconLabel->setObjectName("iconPreview");

    // Set object names
    m_urlInput->setObjectName("formInput");
    m_urlInput->setPlaceholderText("*Required*");
    m_titleInput->setObjectName("formInput");
    m_titleInput->setPlaceholderText("*Required*");

    m_usernameInput->setObjectName("formInput");
    m_usernameInput->setPlaceholderText("Optional");
    m_usernameInput->setToolTip("Store username for quick access. This feature is implemented solely as a convenience to the user.\n Please leave blank if you prefer to use your own credentials manager");

    m_passwordInput->setObjectName("formInput");
    m_passwordInput->setPlaceholderText("Optional");
    m_passwordInput->setToolTip("Store password for quick access. This feature is implemented solely as a convenience to the user.\n Please leave blank if you prefer to use your own credentials manager");

    m_commentsInput->setObjectName("formTextArea");
    m_commentsInput->setPlaceholderText("Optional");

    // Create layouts for username/password with eye buttons
    QHBoxLayout* usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(m_usernameInput);
    usernameLayout->addWidget(m_usernameEyeButton);
    usernameLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(m_passwordInput);
    passwordLayout->addWidget(m_passwordEyeButton);
    passwordLayout->setContentsMargins(0, 0, 0, 0);

    // Add to form
    formLayout->addRow("Icon:", m_iconLabel);
    formLayout->addRow("URL:", m_urlInput);
    formLayout->addRow("Title:", m_titleInput);
    formLayout->addRow("Username:", usernameLayout);
    formLayout->addRow("Password:", passwordLayout);
    formLayout->addRow("Comments:", m_commentsInput);

    // Connect eye button signals
    connect(m_usernameEyeButton, &QPushButton::toggled, [this](bool checked) {
        m_usernameInput->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(m_passwordEyeButton, &QPushButton::toggled, [this](bool checked) {
        m_passwordInput->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    layout->addLayout(formLayout);

    // Create action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    // Create buttons
    m_addButton = new QPushButton("Add");
    m_addButton->setToolTip("Save the website to your collection");

    m_editButton = new QPushButton("Upd");
    m_editButton->setToolTip("Update website if changes were made");

    m_deleteButton = new QPushButton("Del");
    m_deleteButton->setToolTip("Delete the selected website");

    m_launchButton = new QPushButton("Launch");
    m_launchButton->setToolTip("Launch the selected website");

    m_clearButton = new QPushButton("Clear");
    m_clearButton->setToolTip("Clear the form");


    // Set icons
    //m_addButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    //m_editButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    //m_deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    //m_launchButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    //m_clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));

    // Set object names for styling
    m_addButton->setObjectName("primaryButton");
    m_launchButton->setObjectName("primaryButton");
    m_editButton->setObjectName("secondaryButton");
    m_deleteButton->setObjectName("secondaryButton");
    m_clearButton->setObjectName("secondaryButton");

    // Initially disable edit/delete/launch buttons
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_launchButton->setEnabled(false);

    // Add buttons to layout
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_launchButton);
    buttonLayout->addWidget(m_clearButton);

    // Add some spacing before buttons
    layout->addSpacing(16);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    // Connect buttons to their slots
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddWebsite);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditWebsite);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteWebsite);
    connect(m_launchButton, &QPushButton::clicked, this, &MainWindow::onLaunchWebsite);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearForm);


    return panel;
}

QWidget* MainWindow::createWebViewContainer() {
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    // Add URLBar
    layout->addWidget(m_urlBar);
    //m_urlBar->setVisible(true);
    // Create tab widget
    m_tabWidget = new QTabWidget();

    // connect tab switching to update url in urlbar
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0) {
            if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(index))) {
                m_urlBar->setUrl(view->url().toString());
                //refresh to solve resize issues
                // Fix WebView display issues
                view->updateGeometry();
                view->update();
            }
        }
    });



    connect(page, &MyWebPage::newTabRequested, this, [this](QWebEngineView *view, QWebEngineProfile *){
        // Just use your existing method
         createNewTabWithUrlFromLink(view->url().toString(), view);
    });


    connect(page, &MyWebPage::newPopupRequested, this, [this](QWebEngineView *view, QWebEngineProfile *){
            createNewTabWithUrlFromLink(view->url().toString(), view);

        });



    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setMovable(true);

    // Add widgets to layout
    layout->addWidget(m_tabWidget, 1);

    return container;
}

QFrame* MainWindow::createWebsiteCard(const Website& website, int index) {
    QFrame* card = new QFrame();
    card->setObjectName(QString("websiteCard_%1").arg(index));
    card->setProperty("index", index);
    card->setFrameShape(QFrame::StyledPanel);
    card->setCursor(Qt::PointingHandCursor);
    card->setObjectName("websiteCard");
    card->setFixedSize(200, 200); // Square cards - was 180
    //card->setMinimumSize(180, 180); // Set minimum instead
    //card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    card->setCursor(Qt::ArrowCursor);  // Always arrow, never hand


    // Add shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15); // was 12

    // Create a horizontal layout for title and favicon
    QHBoxLayout* titleLayout = new QHBoxLayout();

    // Add favicon
    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(16, 16);

    if (!website.favicon.isNull()) {
        // Use the stored favicon
        iconLabel->setPixmap(website.favicon.pixmap(16, 16));
    } else {
        // Use a default icon
        iconLabel->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));
    }

    titleLayout->addWidget(iconLabel);

    // Title
    QLabel* titleLabel = new QLabel(website.title);
    titleLabel->setObjectName("cardTitle");
    titleLabel->setWordWrap(true);
    titleLayout->addWidget(titleLabel, 1); // 1 gives it stretch priority

    // Add title layout to main layout
    layout->addLayout(titleLayout);

    // URL
    QLabel* urlLabel = new QLabel(website.url);
    urlLabel->setObjectName("cardUrl");
    urlLabel->setWordWrap(true);
    layout->addWidget(urlLabel);


    // Add action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    // Launch button
    launchBtn = new QPushButton();
    launchBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    launchBtn->setToolTip("Launch Website");
    launchBtn->setFlat(true);
    launchBtn->setCursor(Qt::PointingHandCursor);
    launchBtn->setObjectName("cardButton");

    // Edit button
    editBtn = new QPushButton();
    editBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    editBtn->setToolTip("Update Website Details");
    editBtn->setFlat(true);
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setObjectName("cardButton");

    // Delete button
    deleteBtn = new QPushButton();
    deleteBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    deleteBtn->setToolTip("Delete Website");
    deleteBtn->setFlat(true);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setObjectName("cardButton");

    // Add buttons to layout
    buttonLayout->addWidget(launchBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    // Connect button signals
    connect(launchBtn, &QPushButton::clicked, [this, index]() {
        m_websiteList->setCurrentIndex(m_model->index(index, 0));
        onLaunchWebsite();
    });

    connect(editBtn, &QPushButton::clicked, [this, index]() {
        //m_websiteList->setCurrentIndex(m_model->index(index, 0));
        //onWebsiteSelected(m_model->index(index, 0));
        onEditWebsite();
    });

    connect(deleteBtn, &QPushButton::clicked, [this, index]() {
        m_websiteList->setCurrentIndex(m_model->index(index, 0));
        onDeleteWebsite();
    });

    // Connect card click
    card->installEventFilter(this);
    card->setProperty("websiteIndex", index);

    return card;
}

QFrame* MainWindow::createSessionCard(const QString& sessionName, const SessionData& session) {
    QFrame* card = new QFrame();
    card->setObjectName(QString("sessionCard_%1").arg(sessionName));
    card->setProperty("sessionName", sessionName);
    card->setFrameShape(QFrame::StyledPanel);
    card->setCursor(Qt::PointingHandCursor);
    card->setObjectName("sessionCard");
    card->setFixedSize(200, 200); // Square cards - was 180
    card->setCursor(Qt::ArrowCursor);  // Always arrow, never hand

    //card->installEventFilter(this);

    // Add shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15); // was  12

    // Title row with icon and name
    QHBoxLayout* titleLayout = new QHBoxLayout();

    // Icon display
    QLabel* iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(20, 20); // was 24
    iconLabel->setScaledContents(true);
    if (!session.icon.isNull()) {
        QPixmap pixmap = session.icon.pixmap(20, 20); // was 24
        iconLabel->setPixmap(pixmap);
    }
    titleLayout->addWidget(iconLabel);

    // Session name
    QLabel* nameLabel = new QLabel(sessionName);
    nameLabel->setObjectName("cardTitle");
    nameLabel->setWordWrap(true);

    // OPTION: Smaller font
    QFont font = nameLabel->font();
    font.setPointSize(8); // Slightly smaller
    nameLabel->setFont(font);

    titleLayout->addWidget(nameLabel);
    titleLayout->addStretch();

    layout->addLayout(titleLayout);

    // Tab count
    QLabel* tabCountLabel = new QLabel(QString("%1 tabs").arg(session.openTabUrls.count()));
    tabCountLabel->setObjectName("cardTabCount");
    layout->addWidget(tabCountLabel);

    // Add action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    // Load button
    QPushButton* loadBtn = new QPushButton();
    loadBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    loadBtn->setToolTip("Load Session");
    loadBtn->setFlat(true);
    loadBtn->setCursor(Qt::PointingHandCursor);
    loadBtn->setObjectName("cardButton");

    // Delete button
    QPushButton* deleteBtn = new QPushButton();
    deleteBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    deleteBtn->setToolTip("Delete Session");
    deleteBtn->setFlat(true);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setObjectName("cardButton");

    // Add buttons to layout
    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    // Connect button signals
    connect(loadBtn, &QPushButton::clicked, [this, sessionName]() {
        onSessionSelected(sessionName);
        onLoadSelectedSession();
    });

    connect(deleteBtn, &QPushButton::clicked, [this, sessionName]() {
        onSessionSelected(sessionName);

        onDeleteSession();
    });




    card->installEventFilter(this);
    card->setProperty("sessionName", sessionName);

    return card;
}



void MainWindow::updateWebsiteCards() {
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_websitesGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_websiteCards.clear();

    // Add website cards
    int row = 0;
    int col = 0;
    int maxCols = 3; // Adjust based on your preference

    for (int i = 0; i < m_model->rowCount(); ++i) {
        Website website = m_model->getWebsite(i);
        QFrame* card = createWebsiteCard(website, i);
        m_websitesGrid->addWidget(card, row, col);
        m_websiteCards[i] = card;

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
    // Add a stretch at the end
    m_websitesGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);


    // Restore highlight for currently selected website
    QModelIndex currentSelection = m_websiteList->currentIndex();
    if (currentSelection.isValid()) {
        onWebsiteSelected(currentSelection);
    }
}

void MainWindow::updateSessionCards() {
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_sessionsGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_sessionCards.clear();

    // Add session cards
    int row = 0;
    int col = 0;
    int maxCols = 3; // Adjust based on your preference

    QStringList sessionNames = getAvailableSessions();
    for (const QString& sessionName : sessionNames) {
        if (m_sessions.contains(sessionName)) {
            QFrame* card = createSessionCard(sessionName, m_sessions[sessionName]);
            m_sessionsGrid->addWidget(card, row, col);
            m_sessionCards[sessionName] = card;

            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }

    // Add a stretch at the end
    m_sessionsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);
}

void MainWindow::highlightCard(QFrame* card, bool highlight) {
    if (!card) return;

    if (highlight) {
        card->setProperty("selected", true);
        card->style()->unpolish(card);
        card->style()->polish(card);
        card->update();
    } else {
        card->setProperty("selected", false);
        card->style()->unpolish(card);
        card->style()->polish(card);
        card->update();
    }
}

void MainWindow::setupModernStyles() {
    // This is a fallback if the external stylesheet can't be loaded
    setStyleSheet(
        // Main application
        "QMainWindow { background-color: #f5f5f5; }"

        // Tab widget
        "QTabWidget::pane { border: none; }"
        "QTabWidget::tab-bar { alignment: left; }"
        "QTabBar::tab { padding: 8px 16px; background: #e0e0e0; border: none; }"
        "QTabBar::tab:selected { background: #2196F3; color: white; }"
        "QTabBar::tab:hover:!selected { background: #d0d0d0; }"

        // Website cards
        "#websiteCard {"
        "  background-color: white;"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "}"
        "#websiteCard:hover {"
        "  background-color: #f0f7ff;"
        "}"
        "#websiteCard[selected=true] {"
        "  background-color: #e3f2fd;"
        "  border: 2px solid #2196F3;"
        "}"

        // Session cards
        "#sessionCard {"
        "  background-color: white;"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "}"
        "#sessionCard:hover {"
        "  background-color: #f0f7ff;"
        "}"

        // Card elements
        "#cardTitle { font-weight: bold; font-size: 16px; }"
        "#cardUrl { color: #2196F3; font-size: 12px; }"
        "#cardComments { color: #757575; font-size: 12px; margin-top: 8px; }"
        "#cardTime { color: #757575; font-size: 12px; }"
        "#cardTabCount { color: #2196F3; font-size: 14px; font-weight: bold; }"
        "#cardButton { border: none; padding: 4px; }"
        "#cardButton:hover { background-color: #e0e0e0; border-radius: 4px; }"

        // Details panel
        "#detailsPanel {"
        "  background-color: #f5f5f5;"
        "  border-radius: 8px;"
        "  margin: 8px;"
        "}"
        "#detailsPanelTitle { font-size: 18px; font-weight: bold; margin-bottom: 16px; }"

        // Form elements
        "#formInput, #formTextArea {"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  background-color: white;"
        "}"
        "#formInput:focus, #formTextArea:focus {"
        "  border: 2px solid #2196F3;"
        "}"

        // Buttons
        "#primaryButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "}"
        "#primaryButton:hover {"
        "  background-color: #1976D2;"
        "}"
        "#primaryButton:disabled {"
        "  background-color: #B3E5FC;"
        "  color: #E1F5FE;"
        "}"
        "#secondaryButton {"
        "  background-color: #f5f5f5;"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "}"
        "#secondaryButton:hover {"
        "  background-color: #e0e0e0;"
        "}"

        // Dashboard button
        "#dashboardButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "  margin: 8px;"
        "}"
        "#dashboardButton:hover {"
        "  background-color: #1976D2;"
        "}"
        );
}

void MainWindow::onAddWebsite() {
    // Get the data from the form
    QString url = m_urlInput->text().trimmed();
    QString title = m_titleInput->text().trimmed();
    QString userName = m_usernameInput->text().trimmed();
    QString userPassword = m_passwordInput->text().trimmed();

    QString comments = m_commentsInput->toPlainText().trimmed();

    // Validate URL
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Invalid URL", "Please enter a valid URL.");
        return;
    }

    // Add http:// if missing
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
        m_urlInput->setText(url);
    }

    // If title is empty, use the URL as title
    if (title.isEmpty()) {
        title = url;
        m_titleInput->setText(title);
    }

    // Add the website to the model
    Website website;
    website.url = url;
    website.title = title;
    website.username = userName;
    website.password = userPassword;
    website.comments = comments;
    m_model->addWebsite(website);
    // Clear the form
    onClearForm();
    // Save the model
    saveModel();
    // Update the website cards
    updateWebsiteCards();
    // Show a status message
    statusBar()->showMessage("Website added successfully.", 3000);
}


void MainWindow::onEditWebsite() {
    // Get the selected index
    QModelIndex currentIndex = m_websiteList->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "No Selection", "Please select a website to edit.");
        return;
    }

    // Get the data from the form
    QString url = m_urlInput->text().trimmed();
    QString title = m_titleInput->text().trimmed();
    QString userName = m_usernameInput->text().trimmed();
    QString userPassword = m_passwordInput->text().trimmed();
    QString comments = m_commentsInput->toPlainText().trimmed();

    // Validate URL
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Invalid URL", "Please enter a valid URL.");
        return;
    }

    // Add http:// if missing
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
        m_urlInput->setText(url);
    }

    // If title is empty, use the URL as title
    if (title.isEmpty()) {
        title = url;
        m_titleInput->setText(title);
    }

    // Get favicon from the icon label
    QIcon favicon;
    if (!m_iconLabel->pixmap().isNull()) {
        favicon = QIcon(m_iconLabel->pixmap());
    }

    // Update the website in the model
    Website website;
    website.url = url;
    website.title = title;
    website.username = userName;
    website.password = userPassword;
    website.comments = comments;
    website.favicon = favicon;  // Preserve the favicon from icon label

    m_model->setWebsite(currentIndex.row(), website);

    // Clear the form
    onClearForm();

    // Save the model
    saveModel();

    // Update the website cards
    updateWebsiteCards();

    // Show a status message
    statusBar()->showMessage("Website updated successfully.", 3000);
}

void MainWindow::onDeleteWebsite() {
    // Get the selected index
    QModelIndex currentIndex = m_websiteList->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "No Selection", "Please select a website to delete.");
        return;
    }

    // Ask for confirmation
    if (QMessageBox::question(this, "Confirm Deletion",
                              "Are you sure you want to delete this website?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // Delete the website from the model
        m_model->removeWebsite(currentIndex.row());

        // Clear the form
        onClearForm();

        // Save the model
        saveModel();

        // Update the website cards
        updateWebsiteCards();

        // Show a status message
        statusBar()->showMessage("Website deleted successfully.", 3000);
    }
}

void MainWindow::onLaunchWebsite() {

    QModelIndex index = m_websiteList->currentIndex();

    if (!index.isValid()) {
        QMessageBox::information(this, "Selection Required", "Please select a website to launch.");
        return;
    }

    Website website = m_model->getWebsite(index.row());

    // Create a new web view for the website
    QWebEngineView* webView = new QWebEngineView(m_tabWidget);
    connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
        if (QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            if (sender() == currentView) {
                m_urlBar->setUrl(url.toString());
            }
        }
    });
    // Determine which profile to use
    QWebEngineProfile *profile;

    if (m_usingNamedProfiles && !m_selectedProfileName.isEmpty()) {
        // Use private profile
        profile = getOrCreateNamedProfile(m_selectedProfileName);

        //configureProfile(profile);

        m_tabNamedProfiles[webView] = m_selectedProfileName;

    } else if (m_usingSeparateProfiles) {
        // Use separate profile
        profile = createProfileForTab();

        //configureProfile(profile);

        m_tabProfiles[webView] = profile;

    } else {
        // Use shared profile
        profile = m_webProfile;
    }

    // Create page with the selected profile
    page = new MyWebPage(profile, webView);

    // full screen
    configurePage(page);

    webView->setPage(page);
    setupCustomContextMenu(webView, profile);

    // Add new tab for this website FIRST

    int tabIndex = m_tabWidget->addTab(webView, website.title);

    // Set the tab icon if we already have a favicon
    if (!website.favicon.isNull()) {
        m_tabWidget->setTabIcon(tabIndex, website.favicon);
    }
    showWebViews();
    // Set the current tab
    m_tabWidget->setCurrentIndex(tabIndex);

    // NOW load the URL (after switching views)
    webView->setUrl(QUrl(website.url));

    // Disable the checkbox after opening the first tab
    if (m_tabWidget->count() == 1) {  // Changed from 0 to 1
        //m_separateProfilesCheckbox->setEnabled(false);
    }

    // Connect to the destroyed signal to clean up the profile
    connect(webView, &QObject::destroyed, this, [this, webView]() {
        cleanupTabProfile(webView);
    });

    connect(webView, &QWebEngineView::iconChanged, this, [this, websiteIndex = index.row(), webView](const QIcon &icon) {

        updateWebsiteIcon(websiteIndex, icon);

        // Update the tab icon
        int tabIndex = m_tabWidget->indexOf(webView);
        if (tabIndex != -1) {
            m_tabWidget->setTabIcon(tabIndex, icon);
        }

        // Update website cards if on dashboard
        if (m_stackedWidget->currentWidget() == m_dashboardWidget) {

            updateWebsiteCards();

        }
        // Disconnect ALL iconChanged signals from this webView to this MainWindow
        disconnect(webView, &QWebEngineView::iconChanged, this, nullptr);
    });
    statusBar()->showMessage("Launched: " + website.title, 3000);
}

void MainWindow::onClearForm() {
    // Clear the form fields
    m_urlInput->clear();
    m_titleInput->clear();
    m_usernameInput->clear();
    m_passwordInput->clear();
    m_commentsInput->clear();
    m_iconLabel->clear();

    // Deselect any selected website
    m_websiteList->clearSelection();

    // Disable edit/delete/launch buttons
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_launchButton->setEnabled(false);

    // Unhighlight all cards
    for (QFrame* card : m_websiteCards.values()) {
        highlightCard(card, false);
    }
}

void MainWindow::onWebsiteSelected(const QModelIndex &index) {
    if (index.isValid()) {
        m_currentWebsiteIndex = index.row();

        // Get the website
        Website website = m_model->getWebsite(index.row());

        // Populate the form
        populateFormFromWebsite(website);

        // Enable edit/delete/launch buttons
        m_editButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
        m_launchButton->setEnabled(true);

        // Highlight the selected card and unhighlight others
        for (int i = 0; i < m_model->rowCount(); ++i) {
            if (m_websiteCards.contains(i)) {
                highlightCard(m_websiteCards[i], i == index.row());
            }
        }
    }
}

void MainWindow::onWebsiteDoubleClicked(const QModelIndex &index) {
    if (index.isValid()) {
        onLaunchWebsite();
    }
}


void MainWindow::onTabCloseRequested(int index) {
    // Get the web view to be closed
    QWidget* widget = m_tabWidget->widget(index);
    QWebEngineView* webView = qobject_cast<QWebEngineView*>(widget);




    // Clean up the profile if this specific tab has a separate profile
    if (webView && m_tabProfiles.contains(webView)) {
        // This tab actually has a separate profile - clean it up
        cleanupTabProfile(webView);
    }

    // Remove named profile reference if this tab uses a named profile
    if (webView && m_tabNamedProfiles.contains(webView)) {
        // Just remove the reference - don't delete the profile itself
        m_tabNamedProfiles.remove(webView);
    }

    // Remove the tab
    m_tabWidget->removeTab(index);

    // Delete the widget with a short delay to allow pending operations to complete
    if (widget) {
        widget->deleteLater();
    }

    // Update tab count in status bar
    updateTabCountStatus();

    // If no tabs left, go back to dashboard
    if (m_tabWidget->count() == 0) {
        showDashboard();
        m_toggleViewAction->setText("Tabs: 0");
        setWindowTitle("Jasmine");
    } else {
        QTimer::singleShot(10, this, [this]() {
            if (QWebEngineView* currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
                // Update WebView geometry and display
                currentView->updateGeometry();
                currentView->update();

                // Update URL bar
                if (m_urlBar) {
                    m_urlBar->setUrl(currentView->url().toString());
                }

                // Update window title
                QString title = currentView->title();
                if (!title.isEmpty()) {
                    setWindowTitle(title + " - Jasmine");
                } else {
                    setWindowTitle("Jasmine - Web Browser");
                }

                // Give focus to the current tab
                currentView->setFocus();

                // Update container if needed
                if (m_webViewContainer) {
                    m_webViewContainer->updateGeometry();
                    m_webViewContainer->update();
                }
            }
        });
    }
}



void MainWindow::populateFormFromWebsite(const Website &website) {
    m_urlInput->setText(website.url);
    m_titleInput->setText(website.title);
    m_usernameInput->setText(website.username);
    m_passwordInput->setText(website.password);
    m_commentsInput->setPlainText(website.comments);

    // Display the icon
    if (!website.favicon.isNull()) {
        m_iconLabel->setPixmap(website.favicon.pixmap(32, 32));
    } else {
        m_iconLabel->clear();
    }
}


void MainWindow::showDashboard() {
    // Save current web view size before switching
    if (m_stackedWidget->currentWidget() == m_webViewContainer) {
        m_savedWebViewSize = this->size();
    }

    // Switch to dashboard
    m_stackedWidget->setCurrentWidget(m_dashboardWidget);

    if (this->isMaximized()) {
        this->setWindowState(Qt::WindowNoState);
    }

    // Lock dashboard size
    this->setFixedSize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);

    m_stackedWidget->update();

    //m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-right.svg"));
    //m_toggleViewAction->setIcon(QIcon(":/resources/icons/repeat.svg"));

    m_toggleViewAction->setToolTip("Switch to Webview");
    setWindowTitle("Jasmine");

    m_urlBar->setVisible(false); // Hide URLBar when in dashboard

    m_toggleViewAction->setEnabled(true);
    updateTabCountStatus();
}

void MainWindow::showWebViews() {
    // Switch to web view
    m_stackedWidget->setCurrentWidget(m_webViewContainer);

    // Allow resizing in web view
    this->setMinimumSize(0, 0);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    // Restore previous web view size if available, otherwise use dashboard size
    if (!m_savedWebViewSize.isEmpty()) {
        this->resize(m_savedWebViewSize);
    } else {
        this->resize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);
    }

    m_stackedWidget->update();

    updateUrlBarState();

    //m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-left.svg"));
    //m_toggleViewAction->setIcon(QIcon(":/resources/icons/repeat.svg"));

    m_toggleViewAction->setToolTip("Switch to Dashboard");
    setWindowTitle("Jasmine - Web Browser");

    m_toggleViewAction->setEnabled(true);
    updateTabCountStatus();
    QWebEngineView* currentView = getCurrentWebView();
    if (currentView) {
        QTimer::singleShot(0, this, [this, currentView]() {
            currentView->updateGeometry();
            currentView->update();
        });
    }
}


QWebEngineView* MainWindow::getCurrentWebView() const {
    if (m_tabWidget && m_tabWidget->currentWidget()) {
        return qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget());
    }
    return nullptr;
}


void MainWindow::updateReturnToWebViewButton() {
    bool hasOpenTabs = m_tabWidget->count() > 0;
    //m_returnToWebViewButton->setVisible(hasOpenTabs);
    if (hasOpenTabs) {
        //m_returnToWebViewButton->setText(QString("Return to Web View (%1 tabs)").arg(m_tabWidget->count()));
    }
}

void MainWindow::saveModel() {
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/websites.dat";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);

        // Get the list of websites
        QList<Website> websites = m_model->getWebsites();

        // Write the count first
        out << quint32(websites.size());

        // Write each website individually
        for (const Website &website : websites) {
            out << website;
        }

        file.close();
    }
}

WebsiteListModel* MainWindow::loadModel() {
    WebsiteListModel* model = new WebsiteListModel(this);

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/websites.dat";
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);

        // Read the count
        quint32 count;
        in >> count;

        // Read each website
        QList<Website> websites;
        for (quint32 i = 0; i < count; ++i) {
            Website website;
            in >> website;
            websites.append(website);
        }

        model->setWebsites(websites);
        file.close();
    }

    return model;
}

QString MainWindow::getWebsitesFilePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/websites.dat";
}


void MainWindow::createMenus() {
    // Create menus
    QMenu* fileMenu = menuBar()->addMenu("File");
    QMenu* editMenu = menuBar()->addMenu("Edit");
    QMenu* viewMenu = menuBar()->addMenu("View");
    QMenu* navigateMenu = menuBar()->addMenu("Navigate");
    QMenu* sessionMenu = menuBar()->addMenu("Sessions");
    QMenu* toolsMenu = menuBar()->addMenu("Tools");
    //QMenu* helpMenu = menuBar()->addMenu("Help");

    //security menu
    m_securityManager->setupSecurityMenu(menuBar());
    QMenu* helpMenu = menuBar()->addMenu("Help");

    // Add existing toolbar actions to menus
    fileMenu->addAction(m_addCurrentSessionAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_addWebsiteFromUrlAction);
    fileMenu->addAction(m_screenshotAction);


    fileMenu->addSeparator();

    editMenu->addAction(m_copyUrlAction);
    editMenu->addSeparator();
    editMenu->addAction(m_addCurrentWebsiteAction);

    // Create zoom submenu
    QMenu* zoomSubmenu = viewMenu->addMenu("Zoom");

    // Create separate zoom actions for menu only
    QAction* menuZoomInAction = new QAction(QIcon(":/resources/icons/zoom-in.svg"), "Zoom In", this);
    QAction* menuZoomOutAction = new QAction(QIcon(":/resources/icons/zoom-out.svg"), "Zoom Out", this);

    // Set shortcuts for menu zoom actions
    menuZoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    menuZoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));
    // Add actions to main window for global shortcuts
    this->addAction(menuZoomInAction);
    this->addAction(menuZoomOutAction);

    zoomSubmenu->addAction(menuZoomInAction);
    zoomSubmenu->addAction(menuZoomOutAction);

    // Connect menu zoom actions
    connect(menuZoomInAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            view->setZoomFactor(view->zoomFactor() + 0.1);
        }
    });
    connect(menuZoomOutAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            view->setZoomFactor(view->zoomFactor() - 0.1);
        }
    });

    viewMenu->addSeparator();
    viewMenu->addAction(m_toggleViewAction);
    viewMenu->addAction(m_toggleUrlBarAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_downloadsAction);
    viewMenu->addAction(m_closeAllTabsAction);
    viewMenu->addSeparator();

    highlightButtonsAction = new QAction(this);
    highlightButtonsAction->setText("Highlight Essential Buttons");

    highlightButtonsAction->setCheckable(true);
    highlightButtonsAction->setChecked(buttonsHighlighted);

    connect(highlightButtonsAction, &QAction::triggered, this, [this](bool checked){
        QSettings settings;
        settings.setValue("buttonsHighlighted", checked);
        highlightButtonsAction->setChecked(checked);
        buttonsHighlighted = checked;
        highlightButtons(checked);

        settings.sync();
    });
    viewMenu->addAction(highlightButtonsAction);

    navigateMenu->addAction(m_webBackAction);
    navigateMenu->addAction(m_webForwardAction);
    navigateMenu->addAction(m_webReloadAction);
    navigateMenu->addAction(m_goHomeAction);

    // Create new actions for sessions menu
    QAction* saveSessionAction = sessionMenu->addAction("Save Current Session");
    QAction* loadSessionAction = sessionMenu->addAction("Load Session");
    QAction* manageSessionsAction = sessionMenu->addAction("Manage Sessions");
    sessionMenu->addSeparator();
    QAction* cleanAllDataAction = sessionMenu->addAction("Clean Current Sessions' Data");
    QAction* cleanSharedDataAction = sessionMenu->addAction("Clean Shared Profile Data");
    QAction* factoryResetAction = new QAction("Restore Factory Defaults", this);
    sessionMenu->addAction(factoryResetAction);

    // Create new actions for tools menu
    QAction* separateProfilesAction = toolsMenu->addAction("Toggle Private Profile");
    QAction* themeAction = toolsMenu->addAction("Toggle Theme");
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_open2faManagerAction);

    // Create new actions for other menus
    QAction* exitAction = fileMenu->addAction("Exit");
    QAction* aboutAction = helpMenu->addAction("About");
    QAction* featuresAction = helpMenu->addAction("Features");
    QAction* instructionsAction = helpMenu->addAction("How to Use");
    QAction* securityAction = helpMenu->addAction("Security");
    QAction* twofaAction = helpMenu->addAction("2FA Utility");
    QAction* dataManagementAction = helpMenu->addAction("Data Management");
    QAction* downloadManagementAction = helpMenu->addAction("Download Management");
    QAction* onSitesandSessiosAction = helpMenu->addAction("On Sites and Sessions");
    QAction* onSecurityAction = helpMenu->addAction("On Security");
    QAction* onNewStorageAction = helpMenu->addAction("On the New Storage System");
    QAction* onNamedProfilesAction = helpMenu->addAction("On Named Shared Profiles");
    QAction* showChangelogAction = helpMenu->addAction("Changelog");

    QAction* supportusAction = helpMenu->addAction("Support Us");

    // Set shortcuts
    exitAction->setShortcut(QKeySequence::Quit);

    // Connect new session actions
    connect(saveSessionAction, &QAction::triggered, this, &MainWindow::onSaveSession);
    connect(loadSessionAction, &QAction::triggered, this, &MainWindow::onLoadSession);
    connect(manageSessionsAction, &QAction::triggered, this, &MainWindow::onManageSessions);
    connect(cleanAllDataAction, &QAction::triggered, this, &MainWindow::onCleanAllData);
    connect(cleanSharedDataAction, &QAction::triggered, this, &MainWindow::onCleanSharedProfileData);
    connect(factoryResetAction, &QAction::triggered, this, &MainWindow::onRestoreFactoryDefaults);

    // Connect tools actions
    connect(separateProfilesAction, &QAction::triggered, [this]() {
        m_separateProfilesToggle->toggle();
    });
    connect(themeAction, &QAction::triggered, [this]() {
        m_themeToggle->toggle();
    });

    // Connect other actions
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(aboutAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::About, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(featuresAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Features, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(instructionsAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Instructions, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(securityAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Security, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(twofaAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::TwoFA, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(dataManagementAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::DataManagement, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(downloadManagementAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::DownloadManagement, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(onSitesandSessiosAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onSitesAndSessions, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(onSecurityAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onSecurity, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });

    connect(onNewStorageAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onNewStorageSystem, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });
    connect(onNamedProfilesAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onNamedProfiles, this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }
    });

    connect(showChangelogAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onChangelog);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }

    });

    connect(supportusAction, &QAction::triggered, [this]() {
        DonationDialog dialog(this);
        if(m_isDarkTheme){
            m_themeToggle->toggle();
            dialog.exec();
            m_themeToggle->toggle();
        }else{
            dialog.exec();
        }

    });
    // Cleaup at app startup checkbox
    toolsMenu->addSeparator();
    QAction* cleanupTempProfilesAction = toolsMenu->addAction("Clean Temporary Profiles on Startup");
    cleanupTempProfilesAction->setCheckable(true);

    // Load setting and set checkbox state
    QSettings settings;
    bool cleanupEnabled = settings.value("cleanupTempProfilesOnStartup", false).toBool();
    cleanupTempProfilesAction->setChecked(cleanupEnabled);

    // Connect to save setting when toggled
    connect(cleanupTempProfilesAction, &QAction::triggered, [this, cleanupTempProfilesAction](bool checked) {

        if (checked) {
            QMessageBox::StandardButton reply = QMessageBox::warning(this,
                                                                     tr("Warning"),
                                                                     tr("Enabling this option will remove temporary profile directories, potentially including those from older versions. "
                                                                        "The storage system has changed in this version, and enabling this will result in the loss of your sessions created before this update. "
                                                                        "Are you sure you want to enable this?"),
                                                                     QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                cleanupTempProfilesAction->setChecked(false); // Uncheck the box if the user cancels
                return;
            }
        }
        QSettings settings;
        settings.setValue("cleanupTempProfilesOnStartup", checked);
        settings.sync();
    });
    // AdBlocker toggle
    toolsMenu->addSeparator();
    /*
    QAction *toggleAdBlockAction = toolsMenu->addAction("Toggle Ad Blocker");
    toggleAdBlockAction->setCheckable(true);
    toggleAdBlockAction->setChecked(false);

    connect(toggleAdBlockAction, &QAction::toggled, this, [this](bool checked) {
        m_adBlocker->setEnabled(checked);
    });
    */
    QAction *toggleAdBlockAction = toolsMenu->addAction("Toggle Ad Blocker");
    toggleAdBlockAction->setCheckable(true);

    // Load the saved state
    //QSettings settings;
    bool adBlockEnabled = settings.value("adBlocker/enabled", false).toBool();
    toggleAdBlockAction->setChecked(adBlockEnabled);
    m_adBlocker->setEnabled(adBlockEnabled);  // Apply the loaded setting

    // Connect with triggered signal for checkable action
    connect(toggleAdBlockAction, &QAction::triggered, this, [this](bool checked) {
        m_adBlocker->setEnabled(checked);

        QSettings settings;
        settings.setValue("adBlocker/enabled", checked);
        settings.sync();
    });

    toolsMenu->addSeparator();

    //QAction *disableGpuAction = toolsMenu->addAction("Disable GPU Acceleration");
    QAction *disableGpuAction = toolsMenu->addAction("Disable GPU Acceleration (NVIDIA Fix)");
    disableGpuAction->setCheckable(true);
    disableGpuAction->setToolTip("Fix graphical issues on NVIDIA systems (requires restart)");

    // Load the saved state (defaulting to false/unchecked)

    bool gpuDisabled = settings.value("gpu/disable_acceleration", false).toBool();
    disableGpuAction->setChecked(gpuDisabled);

    // Connect with triggered signal
    connect(disableGpuAction, &QAction::triggered, this, [this](bool checked) {
        QSettings settings;
        settings.setValue("gpu/disable_acceleration", checked);
        settings.sync();

        // Show restart notification
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Restart Required");
            msgBox.setText("This change will take effect after restarting the application.");
        if (checked) {
               msgBox.setInformativeText("GPU acceleration has been disabled. "
                                        "This resolves graphical issues on many NVIDIA systems, "
                                        "but may slightly reduce performance.");
           }

           msgBox.setIcon(QMessageBox::Information);
           msgBox.exec();
    });

    toolsMenu->addSeparator();
    QAction *openRadiosAction = toolsMenu->addAction("Browse Radio Stations");
    connect(openRadiosAction, &QAction::triggered, this, [this](){

        if(radioSearchDialog)  radioSearchDialog->show();

    });

    QAction *updateServersAction = toolsMenu->addAction("Refresh Radio Browser API Servers");
    connect(updateServersAction, &QAction::triggered, [this]() {

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Confirm Refresh",
            "Refresh Radio Browser API servers?\n\nThis will discover available servers again.\nOnly needed if current servers are failing.",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            if (radioSearchDialog && radioSearchDialog->api()) {
                radioSearchDialog->api()->refreshServers();
                statusBar()->showMessage("Radio Browser servers updated.", 3000);
            }
        }
    });
    toolsMenu->addSeparator();
    QAction *browseIPTVAction = new QAction("Browse IPTV Channels...", this);
    connect(browseIPTVAction, &QAction::triggered, this, [this](){
        if(searchIPTVDialog)  searchIPTVDialog->show();

    });
    toolsMenu->addAction(browseIPTVAction);

    toolsMenu->addSeparator();
    QAction *browsePodcastsAction = new QAction("Browse Podcasts...", this);
    connect(browsePodcastsAction, &QAction::triggered, this, [this](){
        if(searchPodcastdialog)  searchPodcastdialog->show();

    });
    toolsMenu->addAction(browsePodcastsAction);

    QAction *importPodcastsAction = new QAction("Import Podcast Subscriptions...", this);
    connect(importPodcastsAction, &QAction::triggered, this, &MainWindow::importPodcastSubscriptions);
    toolsMenu->addAction(importPodcastsAction);

    QAction *exportPodcastsAction = new QAction("Export Podcast Subscriptions...", this);
    connect(exportPodcastsAction, &QAction::triggered, this, &MainWindow::exportPodcastSubscriptions);
    toolsMenu->addAction(exportPodcastsAction);

    QAction *refreshAllPodcastsAction = new QAction("Refresh All Podcasts", this);
    connect(refreshAllPodcastsAction, &QAction::triggered, this, &MainWindow::refreshAllPodcasts);
    toolsMenu->addAction(refreshAllPodcastsAction);
}

QStringList MainWindow::getAvailableSessions() {
    return m_sessions.keys();
}

void MainWindow::deleteSession(const QString& name) {

    if (m_sessions.contains(name)) {
        // Remove from sessions map
        m_sessions.remove(name);

        // Clean up profile directories
        cleanupSessionProfileDirectories(name);

        // Save sessions data
        saveSessionsData();
    }

}

void MainWindow::saveSessionsData() {
    // Save using QSettings for basic data
    QSettings settings;
    settings.beginGroup("Sessions");
    settings.remove("");
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        const QString& name = it.key();
        const SessionData& session = it.value();
        settings.beginGroup(name);
        settings.setValue("timestamp", session.timestamp);
        settings.setValue("tabUrls", session.openTabUrls);
        settings.setValue("tabTitles", session.openTabTitles);
        settings.setValue("usingSeparateProfiles", session.usingSeparateProfiles);
        settings.setValue("comments", session.comments);
        settings.setValue("tabHasSeparateProfile", QVariantList(session.tabHasSeparateProfile.begin(), session.tabHasSeparateProfile.end()));
        settings.setValue("tabOriginalProfileNames", session.tabOriginalProfileNames);

        // Add named profiles data
        settings.setValue("usingNamedProfiles", session.usingNamedProfiles);
        settings.setValue("selectedProfileName", session.selectedProfileName);
        settings.setValue("tabHasNamedProfile", QVariantList(session.tabHasNamedProfile.begin(), session.tabHasNamedProfile.end()));
        settings.setValue("tabNamedProfileNames", session.tabNamedProfileNames);

        settings.endGroup();
    }
    settings.endGroup();

    // Create icons directory
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    QDir().mkpath(iconDir);

    // Save each session's icon and tab icons
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        const QString& name = it.key();
        const SessionData& session = it.value();
        // Save tab icons as PNG files
        for (int i = 0; i < session.openTabIcons.size(); i++) {
            const QIcon& icon = session.openTabIcons[i];
            if (!icon.isNull()) {
                QString tabIconPath = iconDir + name + "_tab_" + QString::number(i) + ".png";
                QPixmap pixmap = icon.pixmap(16, 16);
                pixmap.save(tabIconPath, "PNG");
            }
        }
    }
}


void MainWindow::loadSession(const QString& name) {
    if (!m_sessions.contains(name)) {
        QMessageBox::warning(this, "Session Not Found", "Could not find session: " + name);
        return;
    }

    SessionData session = m_sessions[name];
    // Restore tabs based on their actual profile state
    for (int i = 0; i < session.openTabUrls.size(); i++) {
        QWebEngineView* webView = new QWebEngineView(m_tabWidget);
        connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
            if (QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
                if (sender() == currentView) {
                    m_urlBar->setUrl(url.toString());
                }
            }
        });

        QWebEngineProfile* profile;

        // Check if this tab had a named profile
        bool tabHadNamedProfile = (i < session.tabHasNamedProfile.size()) ?
                                      session.tabHasNamedProfile[i] : false;

        if (tabHadNamedProfile) {
            // This tab used a named profile
            QString profileName = session.tabNamedProfileNames[i];

            profile = getOrCreateNamedProfile(profileName);

            m_tabNamedProfiles[webView] = profileName;
            connect(webView, &QObject::destroyed, this, [this, webView]() {
                m_tabNamedProfiles.remove(webView);
            });
        }
        // Check if tab had separate profile
        else if (i < session.tabHasSeparateProfile.size() && session.tabHasSeparateProfile[i]) {
            // Restore the tab's specific profile
            QString sessionProfileName = "Session_" + name + "_Tab_" + QString::number(i);
            QString sessionProfilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                         "/profiles/" + sessionProfileName;
            profile = new QWebEngineProfile(sessionProfileName, this);

            configureProfile(profile);

            profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
            profile->setPersistentStoragePath(sessionProfilePath);
            profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
            profile->setCachePath(sessionProfilePath + "/cache");

            m_tabProfiles[webView] = profile;

            connect(webView, &QObject::destroyed, this, [this, webView]() {
                cleanupTabProfile(webView);
            });
        }
        else {
            // This tab used the shared profile
            profile = m_webProfile;

        }

        page = new MyWebPage(profile, webView);
        configurePage(page);


        webView->setPage(page);
        setupCustomContextMenu(webView, profile);

        // Add the tab
        int tabIndex = m_tabWidget->addTab(webView, session.openTabTitles[i]);

        // Set favicon if available
        if (i < session.openTabIcons.size() && !session.openTabIcons[i].isNull()) {
            m_tabWidget->setTabIcon(tabIndex, session.openTabIcons[i]);
        }

        // Load the URL
        webView->setUrl(QUrl(session.openTabUrls[i]));

        // Connect iconChanged signal
        connect(webView, &QWebEngineView::iconChanged, [this, tabIndex, webView, name, i](const QIcon &icon) {
            m_tabWidget->setTabIcon(tabIndex, icon);
            if (m_sessions.contains(name) && i < m_sessions[name].openTabIcons.size()) {
                m_sessions[name].openTabIcons[i] = icon;
                saveSessionsData();
            }
        });
    }

    // Update UI state
    if (m_tabWidget->count() > 0) {
        showWebViews();
        m_toggleViewAction->setText("To Dashboard");
        setWindowTitle("Jasmine - Web Browser");
        m_tabWidget->setCurrentIndex(0);  // Set to first tab
    } else {
        showDashboard();
        m_toggleViewAction->setText("To WebView");
        setWindowTitle("Jasmine");
    }

    m_currentSessionName = name;
    statusBar()->showMessage(QString("Session loaded: %1 (%2 tabs)").arg(name).arg(m_tabWidget->count()), 3000);
    updateTabCountStatus();
}



void MainWindow::loadSessionsData() {
    QSettings settings;
    settings.beginGroup("Sessions");
    QStringList sessionNames = settings.childGroups();
    for (const QString& name : sessionNames) {
        settings.beginGroup(name);
        SessionData session;
        session.timestamp = settings.value("timestamp").toDateTime();
        session.name = name;
        session.openTabUrls = settings.value("tabUrls").toStringList();
        session.openTabTitles = settings.value("tabTitles").toStringList();
        session.usingSeparateProfiles = settings.value("usingSeparateProfiles", false).toBool();
        session.comments = settings.value("comments").toString();

        // Load new per-tab profile data
        QVariantList tabHasProfileVariants = settings.value("tabHasSeparateProfile").toList();
        for (const QVariant& variant : tabHasProfileVariants) {
            session.tabHasSeparateProfile.append(variant.toBool());
        }
        session.tabOriginalProfileNames = settings.value("tabOriginalProfileNames").toStringList();

        // Load named profiles data
        session.usingNamedProfiles = settings.value("usingNamedProfiles", false).toBool();
        session.selectedProfileName = settings.value("selectedProfileName", "").toString();

        // Load tab named profile data
        QVariantList tabHasNamedProfileVariants = settings.value("tabHasNamedProfile").toList();
        for (const QVariant& variant : tabHasNamedProfileVariants) {
            session.tabHasNamedProfile.append(variant.toBool());
        }
        session.tabNamedProfileNames = settings.value("tabNamedProfileNames", QStringList()).toStringList();

        // Initialize empty icons list
        for (int i = 0; i < session.openTabUrls.size(); i++) {
            session.openTabIcons.append(QIcon());
        }

        m_sessions[name] = session;
        settings.endGroup();
    }
    settings.endGroup();

    // Load icons from files (existing code)...
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        const QString& name = it.key();
        SessionData& session = it.value();
        // Load session icon
        QString iconPath = iconDir + name + "_session.png";
        if (QFile::exists(iconPath)) {
            session.icon = QIcon(iconPath);
        } else {
            session.icon = generateRandomSvgIcon();
            QPixmap pixmap = session.icon.pixmap(64, 64);
            pixmap.save(iconPath, "PNG");
        }
        // Load tab icons
        session.openTabIcons.clear();
        for (int i = 0; i < session.openTabUrls.size(); i++) {
            QString tabIconPath = iconDir + name + "_tab_" + QString::number(i) + ".png";
            if (QFile::exists(tabIconPath)) {
                session.openTabIcons.append(QIcon(tabIconPath));
            } else {
                session.openTabIcons.append(QIcon());
            }
        }
    }

    updateSessionCards();
}




void MainWindow::onSaveSession(){
    bool ok;
    QString name = QInputDialog::getText(this, "Save Session",
                                         "Enter a name for this session:", QLineEdit::Normal,
                                         m_currentSessionName, &ok);
    if (ok && !name.isEmpty()) {
        // Check if session already exists
        if (m_sessions.contains(name)) {
            QMessageBox::information(this, "Session Exists",
                                     QString("Session '%1' already exists. Please use a different name.").arg(name));
            return; // Don't save, let user try again
        }

        saveSession(name);
        updateSessionCards();
    }
}

void MainWindow::saveSession(const QString& name) {
    if (name.isEmpty()) {
        return;
    }
    if (m_tabWidget->count() == 0) {
        QMessageBox::information(this, "No Tabs", "Cannot save session - no tabs are currently open.");
        return;
    }


    QProgressDialog progressDialog("Saving session...", "Cancel", 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.setCancelButton(nullptr);
    progressDialog.show();
    m_tabWidget->setEnabled(false);
    QApplication::processEvents();

    SessionData session;
    session.timestamp = QDateTime::currentDateTime();
    session.name = name;
    session.usingSeparateProfiles = m_usingSeparateProfiles;
    session.usingNamedProfiles = m_usingNamedProfiles;
    session.selectedProfileName = m_selectedProfileName;

    for (int i = 0; i < m_tabWidget->count(); i++) {
        QWebEngineView* view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        if (view) {
            QWebEngineProfile* profile = view->page()->profile();
            profile->clearHttpCache();
        }
    }

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    for (int i = 0; i < m_tabWidget->count(); i++) {
        QWebEngineView* view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        if (view) {
            session.openTabUrls.append(view->url().toString());
            session.openTabTitles.append(m_tabWidget->tabText(i));
            session.openTabIcons.append(m_tabWidget->tabIcon(i));

            QWebEngineProfile* currentProfile = view->page()->profile();
            if (m_tabNamedProfiles.contains(view)) {
            }

            // First check for named profile
            if (m_tabNamedProfiles.contains(view)) {
                QString profileName = m_tabNamedProfiles[view];
                session.tabNamedProfileNames.append(profileName);
                session.tabHasNamedProfile.append(true);
                session.tabHasSeparateProfile.append(false);
                QWebEngineProfile* namedProfile = m_namedProfiles[profileName];
                QString currentPath = namedProfile->persistentStoragePath();

                QFileInfo pathInfo(currentPath);
                while (pathInfo.isSymLink()) {
                    currentPath = pathInfo.symLinkTarget();
                    pathInfo.setFile(currentPath);
                }

                QString sessionProfileName = "Session_" + name + "_NamedProfile_" + profileName + "_Tab_" + QString::number(i);
                QString sessionProfilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                             "/profiles/sessions/" + sessionProfileName;
                QDir().mkpath(QFileInfo(sessionProfilePath).dir().absolutePath());

                QFileInfo destInfo(sessionProfilePath);
                if (destInfo.exists()) {
                    if (destInfo.isSymLink()) {
                        QFile::remove(sessionProfilePath);
                    } else {
                        QDir(sessionProfilePath).removeRecursively();
                    }
                }

                if (QFile::link(currentPath, sessionProfilePath)) {
                    session.tabOriginalProfileNames.append("");
                } else {
                    qWarning() << "Failed to create symlink for named profile tab" << i
                               << "from" << currentPath << "to" << sessionProfilePath;
                }
            }
            // Then check for separate/private profile
            else if (m_tabProfiles.contains(view)) {
                session.tabHasSeparateProfile.append(true);
                session.tabHasNamedProfile.append(false);
                session.tabNamedProfileNames.append("");

                QWebEngineProfile* currentProfile = m_tabProfiles[view];
                QString currentPath = currentProfile->persistentStoragePath();

                QFileInfo pathInfo(currentPath);
                while (pathInfo.isSymLink()) {
                    currentPath = pathInfo.symLinkTarget();
                    pathInfo.setFile(currentPath);
                }

                QString sessionProfileName = "Session_" + name + "_Tab_" + QString::number(i);
                QString sessionProfilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                             "/profiles/" + sessionProfileName;

                QFileInfo destInfo(sessionProfilePath);
                if (destInfo.exists()) {
                    if (destInfo.isSymLink()) {
                        QFile::remove(sessionProfilePath);
                    } else {
                        QDir(sessionProfilePath).removeRecursively();
                    }
                }

                if (QFile::link(currentPath, sessionProfilePath)) {
                    session.tabOriginalProfileNames.append(currentProfile->storageName());
                } else {
                    qWarning() << "Failed to create symlink for tab" << i
                               << "from" << currentPath << "to" << sessionProfilePath;
                    session.tabOriginalProfileNames.append("");
                }
            }
            // Must be shared profile if not in either map
            else {
                session.tabHasSeparateProfile.append(false);
                session.tabHasNamedProfile.append(false);
                session.tabNamedProfileNames.append("");
                session.tabOriginalProfileNames.append("");
            }
        }
    }

    session.icon = generateRandomSvgIcon();
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    if (!QDir(iconDir).exists()) {
        QDir().mkpath(iconDir);
    }
    QString iconPath = iconDir + name + "_session.png";
    QPixmap pixmap = session.icon.pixmap(64, 64);
    pixmap.save(iconPath, "PNG");

    m_sessions[name] = session;
    m_currentSessionName = name;
    saveSessionsData();
    updateSessionCards();

    m_tabWidget->setEnabled(true);
    progressDialog.close();

    QMessageBox::information(this, "Session Saved",
                             QString("Session '%1' has been saved successfully with %2 tabs.")
                                 .arg(name).arg(m_tabWidget->count()));
    statusBar()->showMessage("Session saved: " + name);
}






void MainWindow::onManageSessions() {
    QStringList sessions = getAvailableSessions();
    if (sessions.isEmpty()) {
        QMessageBox::information(this, "No Sessions", "There are no saved sessions to manage.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Manage Sessions");
    dialog.setMinimumSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QListWidget* sessionList = new QListWidget(&dialog);
    sessionList->addItems(sessions);
    layout->addWidget(sessionList);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* loadButton = new QPushButton("Load", &dialog);
    QPushButton* deleteButton = new QPushButton("Delete", &dialog);
    QPushButton* closeButton = new QPushButton("Close", &dialog);

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    connect(loadButton, &QPushButton::clicked, [&]() {
        QListWidgetItem* item = sessionList->currentItem();
        if (item) {
            loadSession(item->text());
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "No Selection", "Please select a session to load.");
        }
    });

    connect(deleteButton, &QPushButton::clicked, [&]() {
        QListWidgetItem* item = sessionList->currentItem();
        if (item) {
            if (QMessageBox::question(&dialog, "Confirm Deletion",
                                      QString("Are you sure you want to delete the session '%1'?").arg(item->text()),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                deleteSession(item->text());
                delete sessionList->takeItem(sessionList->row(item));

                if (sessionList->count() == 0) {
                    dialog.accept();
                }
            }
        } else {
            QMessageBox::warning(&dialog, "No Selection", "Please select a session to delete.");
        }
    });

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}



void MainWindow::onCleanAllData() {

    // First check if there are any tabs open
    if (m_tabWidget->count() == 0) {
        QMessageBox::information(this, "No Tabs Open",
                                 "There are no tabs currently open to clean.");
        return;
    }

    if (QMessageBox::question(this, "Clean Session Data",
                              "Are you sure you want to clean browsing data from currently loaded sessions?\n"
                              "This will remove cookies, cache, and browsing data from active sessions with Private profiles.\n"
                              "If any tab is using the Shared profile, no cleaning will be performed on it!\n"
                              "You can clean the universal Shared profile in Menu->Sessions->'Clean Shared Profile Data'",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        // Clear only tab profiles (separate profiles)
        for (QWebEngineProfile* profile : m_tabProfiles.values()) {
            if (profile) {  // Check if profile pointer is valid
                profile->clearAllVisitedLinks();
                profile->clearHttpCache();
                if (profile->cookieStore()) {
                    profile->cookieStore()->deleteAllCookies();
                }
            }
        }

        // Show a status message
        statusBar()->showMessage("Session data for separate profiles has been cleared.", 3000);
    }
}




QWebEngineProfile* MainWindow::createProfileForTab(int tabIndex, const QString& sessionName) {
    QString profileName;

    if (!sessionName.isEmpty()) {
        // If we have a session name, use a deterministic name based on that
        profileName = "Session_" + sessionName + "_Tab_" + QString::number(tabIndex);
    } else {
        // Otherwise, use a unique name based on timestamp
        profileName = "TabProfile_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz_") +
                      QString::number(QRandomGenerator::global()->bounded(1000));
    }

    QString profilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                          "/profiles/" + profileName;


    QWebEngineProfile* profile = new QWebEngineProfile(profileName, this);


    configureProfile(profile);

    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->setPersistentStoragePath(profilePath);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setCachePath(profilePath + "/cache");


    return profile;
}





void MainWindow::cleanupTabProfile(QWebEngineView* webView) {
    if (!webView) return;

    // Check if it's a per-tab private profile
    if (m_tabProfiles.contains(webView)) {
        QWebEngineProfile* profile = m_tabProfiles[webView];
        m_tabProfiles.remove(webView);

        bool profileUsedElsewhere = false;
            for (auto it = m_tabProfiles.constBegin(); it != m_tabProfiles.constEnd(); ++it) {
                if (it.value() == profile) {
                    profileUsedElsewhere = true;
                    break;
                }
            }
            if (!profileUsedElsewhere) {
                    profile->deleteLater();
                }

    }

    // Remove named profile mapping (never delete named profiles)
    if (m_tabNamedProfiles.contains(webView)) {
        m_tabNamedProfiles.remove(webView);
    }
}

/*
void MainWindow::cleanupTabProfile(QWebEngineView* webView) {
    if (m_tabProfiles.contains(webView)) {
        QWebEngineProfile* profile = m_tabProfiles[webView];
        QString profileName = profile->storageName();


        // Remove from our map
        m_tabProfiles.remove(webView);

        // NEVER delete profiles during the session - this avoids crashes
        // Just mark them for potential cleanup on next app start
        if (!profileName.startsWith("Session_")) {
            profile->deleteLater();

            // Instead of deleting, we'll just leave it for now
            // We could implement a cleanup mechanism on app start if needed
        } else {
        }
    }
}

*/

void MainWindow::ensureProfileDirectoriesExist() {
    // Make sure the base profiles directory exists
    QString baseProfilesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
    QDir dir(baseProfilesDir);

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create profiles directory:" << baseProfilesDir;
        }
    }

    // Check permissions on the directory
    QFileInfo dirInfo(baseProfilesDir);
    if (!dirInfo.isWritable()) {
        qWarning() << "Profiles directory is not writable:" << baseProfilesDir;
    } else {
    }
}

bool MainWindow::copyProfileData(const QString& sourceDir, const QString& destDir) {

    QDir source(sourceDir);
    QDir dest(destDir);

    // Make sure source exists
    if (!source.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDir;
        return false;
    }

    // Create destination if it doesn't exist
    if (!dest.exists()) {
        if (!dest.mkpath(".")) {
            qWarning() << "Failed to create destination directory:" << destDir;
            return false;
        }
    }

    // Get list of all files and subdirectories
    QFileInfoList entries = source.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    bool success = true;

    // Copy each entry
    for (const QFileInfo& entry : entries) {
        QString srcPath = entry.filePath();
        QString destPath = destDir + "/" + entry.fileName();

        if (entry.isDir()) {
            // Recursively copy subdirectories
            if (!copyProfileData(srcPath, destPath)) {
                success = false;
            }
        } else {
            // Copy file
            if (QFile::exists(destPath)) {
                QFile::remove(destPath);
            }

            // Use QFile directly for better error handling
            QFile srcFile(srcPath);
            if (!srcFile.copy(destPath)) {
                qWarning() << "Failed to copy file:" << srcPath << "to" << destPath;
                qWarning() << "Error:" << srcFile.errorString();
                success = false;
            }
        }
    }

    return success;
}


void MainWindow::cleanupSessionProfileDirectories(const QString& sessionName) {
    // Get the directories to check
    QStringList dirsToCheck;
    // Add the app's profile directory
    QString baseProfilesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
    dirsToCheck.append(baseProfilesDir);
    // Add the Jasmine profiles directory if it's different
    QString webAppsProfilesDir = QDir::homePath() + "/.local/share/Jasmine/profiles";
    if (baseProfilesDir != webAppsProfilesDir) {
        dirsToCheck.append(webAppsProfilesDir);
    }

    for (const QString& dirPath : dirsToCheck) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }

        // Check the sessions subdirectory specifically for named profile symlinks
        QDir sessionsDir(dirPath + "/sessions");
        if (sessionsDir.exists()) {
            QStringList sessionEntries = sessionsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (const QString& entry : sessionEntries) {
                // Match patterns like "Session_g4-home_NamedProfile_home_Tab_0"
                if (entry.startsWith("Session_" + sessionName + "_NamedProfile_")) {
                    QString fullPath = sessionsDir.absoluteFilePath(entry);
                    QFileInfo info(fullPath);
                    bool success = false;
                    if (info.isSymLink()) {
                        // Remove symlink
                        success = QFile::remove(fullPath);
                    } else if (info.isDir()) {
                        // Remove directory
                        QDir entryDir(fullPath);
                        success = entryDir.removeRecursively();
                    }
                    if (!success) {
                        qWarning() << "Failed to remove named profile session link:" << fullPath;
                    }
                }
            }
        }

        // Get all entries in the main directory (both dirs and symlinks)
        QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

        // Find and remove entries matching the original session patterns
        QString tabSessionPrefix = "Session_" + sessionName + "_Tab_";
        QString namedProfileSessionPrefix = "Session_" + sessionName + "_NamedProfile_";

        for (const QString& entry : entries) {
            if (entry.startsWith(tabSessionPrefix) || entry.startsWith(namedProfileSessionPrefix)) {
                QString fullPath = dir.absoluteFilePath(entry);
                QFileInfo info(fullPath);
                bool success = false;
                if (info.isSymLink()) {
                    // Remove symlink
                    success = QFile::remove(fullPath);
                } else if (info.isDir()) {
                    // Remove directory
                    QDir sessionDir(fullPath);
                    success = sessionDir.removeRecursively();
                }
                if (!success) {
                    qWarning() << "Failed to remove:" << fullPath;
                }
            }
        }
    }
}



void MainWindow::prepareForShutdown() {

    // Detach all profiles from our tracking map to prevent double-deletion
    QMap<QWebEngineView*, QWebEngineProfile*> profilesCopy = m_tabProfiles;
    m_tabProfiles.clear();

    // Close all tabs without triggering our normal cleanup
    while (m_tabWidget->count() > 0) {
        QWidget* widget = m_tabWidget->widget(0);
       m_tabWidget ->removeTab(0);
        widget->deleteLater();
    }

    // Wait a moment for tab deletion to complete
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();

}

void MainWindow::listDirectoryContents(const QString& dirPath, int level) {
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    QString indent;
    for (int i = 0; i < level; ++i) {
        indent += "  ";
    }

    for (const QFileInfo& entry : entries) {
        if (entry.isDir()) {
            listDirectoryContents(entry.absoluteFilePath(), level + 1);
        }
    }
}

bool MainWindow::hasOpenTabs() const {
    return m_tabWidget->count() > 0;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    prepareForShutdown();
    QMainWindow::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    // Existing card click handling
    if (event->type() == QEvent::MouseButtonPress) {
        // Handle website card clicks
        if (obj->property("websiteIndex").isValid()) {
            int index = obj->property("websiteIndex").toInt();
            QModelIndex modelIndex = m_model->index(index, 0);
            m_websiteList->setCurrentIndex(modelIndex);
            onWebsiteSelected(modelIndex);
            return true;
        }
        // Handle session card clicks
        if (obj->property("sessionName").isValid()) {
            QString sessionName = obj->property("sessionName").toString();
            onSessionSelected(sessionName);
            return true;
        }
        // Handle radio card clicks
        if (obj->property("radioIndex").isValid()) {
            int index = obj->property("radioIndex").toInt();
            onRadioStationSelected(index);
            return true;
        }

        if (obj->property("iptvIndex").isValid()) {
            int index = obj->property("iptvIndex").toInt();
            onIPTVStationSelected(index);
            return true;
        }

        // Handle podcast card clicks
        if (obj->property("podcastIndex").isValid()) {
            int index = obj->property("podcastIndex").toInt();
            onPodcastShowSelected(index);
            return true;
        }
    }

    // Block maximize attempts in dashboard mode
    if (obj == this) {
        if (event->type() == QEvent::WindowStateChange) {
            if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
                if (this->isMaximized()) {
                    // Immediately restore without timer to reduce flicker
                    this->showNormal();
                    this->resize(1130, 800);
                    return true;
                }
            }
        }
        // Also block resize events that would make window too large in dashboard
        else if (event->type() == QEvent::Resize) {
            if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
                QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
                if (resizeEvent->size().width() > 1130 || resizeEvent->size().height() > 800) {
                    this->resize(1130, 800);
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onLoadSession()
{
    QStringList sessions = getAvailableSessions();

    if (sessions.isEmpty()) {
        QMessageBox::information(this, "No Sessions",
                                 "No saved sessions found.");
        return;
    }

    bool ok;
    QString name = QInputDialog::getItem(this, "Load Session",
                                         "Select a session to load:", sessions, 0, false, &ok);

    if (ok && !name.isEmpty()) {
        //loadSession(name);
        onSessionSelected(name);

        onLoadSelectedSession();
    }
}

void MainWindow::updateWebsiteIcon(int websiteIndex, const QIcon &icon) {
    // Get the current website
    Website website = m_model->getWebsite(websiteIndex);


    // Update the favicon
    website.favicon = icon;

    // Update the website in the model
    m_model->setWebsite(websiteIndex, website);

    // Save the model
    saveModel();

    // Always update the website cards
    // This ensures they'll be up-to-date when switching to the dashboard
    updateWebsiteCards();
    // Also update any open tabs showing this website
    for (int i = 0; i < m_tabWidget->count(); i++) {
        QWebEngineView* webView = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        if (webView && webView->url().toString() == website.url) {
            m_tabWidget->setTabIcon(i, icon);
        }
    }
}

QFrame* MainWindow::createSessionDetailPanel() {
    QFrame* panel = new QFrame();
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setObjectName("detailsPanel");
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);

    layout->addSpacing(16);

    // Title
    QLabel* titleLabel = new QLabel("Session Details");
    titleLabel->setObjectName("detailsPanelTitle");
    layout->addWidget(titleLabel);

    // Form fields
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Icon display
    QHBoxLayout* iconLayout = new QHBoxLayout();
    m_sessionIconLabel = new QLabel();
    m_sessionIconLabel->setObjectName("sessionIconLabel");
    m_sessionIconLabel->setAlignment(Qt::AlignCenter);
    m_sessionIconLabel->setFixedSize(50, 50);
    m_sessionIconLabel->setStyleSheet(
        "QLabel#sessionIconLabel {"
        "    border: 2px solid #ccc;"
        "    border-radius: 8px;"
        "    background-color: #f5f5f5;"
        "}"
        );
    m_sessionIconLabel->setScaledContents(true);

    m_sessionGenerateIconButton = new QPushButton();
    m_sessionGenerateIconButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_sessionGenerateIconButton->setFixedSize(24, 24);
    m_sessionGenerateIconButton->setToolTip("Generate new icon");

    iconLayout->addWidget(m_sessionIconLabel);
    iconLayout->addWidget(m_sessionGenerateIconButton);
    iconLayout->addStretch();

    // Create form fields
    m_sessionNameInput = new QLineEdit();
    m_sessionNameInput->setToolTip("Tip: Using dashes '-' to separate name components improves card appearence");

    m_sessionTabsDisplay = new QListWidget();
    m_sessionTabsDisplay->setMinimumHeight(150);

    // Session timestamp display
    m_sessionTimestampLabel = new QLabel();
    // Set object names for styling
    m_sessionNameInput->setObjectName("formInput");
    m_sessionTabsDisplay->setObjectName("formList");
    // Add fields to form
    formLayout->addRow("Icon:", iconLayout);
    formLayout->addRow("Name:", m_sessionNameInput);
    formLayout->addRow("Created:", m_sessionTimestampLabel);
    formLayout->addRow("Tabs:", m_sessionTabsDisplay);

    layout->addLayout(formLayout);

    // Comments section at the bottom

    layout->addSpacing(16);
    m_sessionCommentsEdit = new QTextEdit();
    m_sessionCommentsEdit->setObjectName("formTextArea");
    m_sessionCommentsEdit->setMinimumHeight(80);
    m_sessionCommentsEdit->setMaximumHeight(120);
    m_sessionCommentsEdit->setPlaceholderText("Add notes or comments about this session...");
    formLayout->addRow("Notes:", m_sessionCommentsEdit);

    // Create action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    // Create buttons
    m_sessionUpdateButton = new QPushButton(" Update");

    m_sessionDeleteButton = new QPushButton(" Delete");
    m_sessionLoadButton = new QPushButton(" Launch");
    m_sessionClearButton = new QPushButton(" Clear");
    m_sessionUpdateButton->setToolTip("Update the selected session");
    m_sessionDeleteButton->setToolTip("Delete the selected session");
    m_sessionLoadButton->setToolTip("Load the selected session");
    m_sessionClearButton->setToolTip("Clear Form");


    // Set icons
    //m_sessionUpdateButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    //m_sessionDeleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    //m_sessionLoadButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    //m_sessionClearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));

    // Set object names for styling
    m_sessionUpdateButton->setObjectName("primaryButton");
    m_sessionLoadButton->setObjectName("primaryButton");
    m_sessionDeleteButton->setObjectName("secondaryButton");
    m_sessionClearButton->setObjectName("secondaryButton");

    // Initially disable edit/delete/load buttons
    m_sessionDeleteButton->setEnabled(false);
    m_sessionLoadButton->setEnabled(false);

    // Add buttons to layout
    buttonLayout->addWidget(m_sessionUpdateButton);
    buttonLayout->addWidget(m_sessionDeleteButton);
    buttonLayout->addWidget(m_sessionLoadButton);
    buttonLayout->addWidget(m_sessionClearButton);

    // Add some spacing before buttons
    layout->addSpacing(16);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    // Connect signals
    connect(m_sessionUpdateButton, &QPushButton::clicked, this, &MainWindow::onUpdateSession);
    connect(m_sessionDeleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSession);
    connect(m_sessionLoadButton, &QPushButton::clicked, this, &MainWindow::onLoadSelectedSession);
    connect(m_sessionClearButton, &QPushButton::clicked, this, &MainWindow::onClearSessionForm);
    connect(m_sessionGenerateIconButton, &QPushButton::clicked, this, [this]() {
        QIcon newIcon = generateRandomSvgIcon();
        QPixmap pixmap = newIcon.pixmap(50, 50);
        m_sessionIconLabel->setPixmap(pixmap);

        // Update the session data if we have a current session
        if (!m_currentSessionName.isEmpty()) {
            m_sessions[m_currentSessionName].icon = newIcon;
        }
    });


    // Connect comments text edit to save changes
    return panel;
}



void MainWindow::populateSessionForm(const SessionData &session) {
    m_sessionNameInput->setText(session.name);
    m_sessionTimestampLabel->setText(session.timestamp.toString("yyyy-MM-dd hh:mm"));
    // Display the session icon
    if (!session.icon.isNull()) {
        QPixmap pixmap = session.icon.pixmap(50, 50);
        m_sessionIconLabel->setPixmap(pixmap);
    } else {
        // Clear the icon if none exists
        m_sessionIconLabel->clear();
        m_sessionIconLabel->setText("No Icon");
        m_sessionIconLabel->setAlignment(Qt::AlignCenter);
    }
    // Display comments
    m_sessionCommentsEdit->setPlainText(session.comments);
    // Clear and populate the tabs list
    m_sessionTabsDisplay->clear();
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    for (int i = 0; i < session.openTabUrls.size(); i++) {
        QString title = session.openTabTitles[i];
        QString url = session.openTabUrls[i];
        // Determine profile type for this tab
        QString profileInfo;
        QString profileTooltip;
        // Check if this tab has a named profile
        bool hasNamedProfile = (i < session.tabHasNamedProfile.size()) ?
                                   session.tabHasNamedProfile[i] : false;
        if (hasNamedProfile) {
            // This tab uses a named profile
            QString profileName = (i < session.tabNamedProfileNames.size()) ?
                                      session.tabNamedProfileNames[i] : "Unknown";
            profileInfo = " [Named Profile: " + profileName + "]";
            profileTooltip = "Named Profile: " + profileName;
        } else {
            // Check if this tab has a separate profile
            bool hasSeparateProfile = (i < session.tabHasSeparateProfile.size()) ?
                                          session.tabHasSeparateProfile[i] : false;
            if (hasSeparateProfile) {
                profileInfo = " [Private Profile]";
                profileTooltip = "Private Profile";
            } else {
                // New logic: if tab has no named profile name, it's always shared
                if (i < session.tabNamedProfileNames.size() && !session.tabNamedProfileNames[i].isEmpty()) {
                    profileInfo = " [Named Profile: " + session.tabNamedProfileNames[i] + "]";
                    profileTooltip = "Named Profile: " + session.tabNamedProfileNames[i];
                } else {
                    profileInfo = " [Shared Profile]";
                    profileTooltip = "Shared Profile";
                }
            }
        }
        // Create item with title, URL, and profile info
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(title + profileInfo + "\n" + url);
        item->setToolTip(url + "\nProfile: " + profileTooltip);
        // Load saved tab icon first
        QString tabIconPath = iconDir + session.name + "_tab_" + QString::number(i) + ".png";
        if (QFile::exists(tabIconPath)) {
            QIcon tabIcon(tabIconPath);
            item->setIcon(tabIcon);
        } else {
            // Try to find a matching website to get its favicon
            for (int j = 0; j < m_model->rowCount(); j++) {
                Website website = m_model->getWebsite(j);
                if (website.url == url && !website.favicon.isNull()) {
                    item->setIcon(website.favicon);
                    break;
                }
            }
        }
        m_sessionTabsDisplay->addItem(item);
    }
    // Enable edit/delete/load buttons
    m_sessionDeleteButton->setEnabled(true);
    m_sessionLoadButton->setEnabled(true);
}



void MainWindow::onSaveCurrentSession()
{
    if(!m_currentSessionName.isEmpty()) {
        onSaveSession();
    }


}

void MainWindow::onDeleteSession()
{
    // Safeguard: Prevent deletion if there are open tabs
    if (m_tabWidget->count() > 0) {
        QMessageBox::warning(this, "Cannot Delete", "Cannot delete a session while tabs are open. Please close all tabs first.");
        return; // Do not proceed with deletion
    }

    QString sessionName = m_sessionNameInput->text();
    if (QMessageBox::question(this, "Delete Session",
                              QString("Are you sure you want to delete the session '%1'?").arg(sessionName),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        deleteSession(sessionName);
        updateSessionCards();
    }
    onClearSessionForm();
}

void MainWindow::onLoadSelectedSession() {
    QString sessionName = m_sessionNameInput->text();

    if (sessionName.isEmpty()) {
        QMessageBox::information(this, "No Session Selected",
                                 "Please select a session to load.");
        return;
    }

    // Check if tabs are already open
    if (m_tabWidget->count() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Tabs Already Open",
                                                                  "There are already open tabs. Do you want to append to the existing session?",
                                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) {
            return;
        } else if (reply == QMessageBox::No) {
            // Close all current tabs
            while (m_tabWidget->count() > 0) {
                onTabCloseRequested(0);
            }
        }
    }

    loadSession(sessionName);
}

void MainWindow::onClearSessionForm() {
    m_sessionNameInput->clear();
    m_sessionTimestampLabel->clear();
    m_sessionIconLabel->clear();
    m_sessionIconLabel->setText("No Icon");
    //m_sessionIconLabel->setAlignment(Qt::AlignCenter);
    m_sessionCommentsEdit->clear();
    m_sessionTabsDisplay->clear();

    // Disable edit/delete/load buttons
    m_sessionUpdateButton->setEnabled(false);
    m_sessionDeleteButton->setEnabled(false);
    m_sessionLoadButton->setEnabled(false);

    // Clear current selection
    m_currentSessionName = "";
}

void MainWindow::onSessionSelected(const QString &sessionName) {
    if (m_sessions.contains(sessionName)) {
        m_currentSessionName = sessionName;
        m_sessionUpdateButton->setEnabled(true);
        m_sessionDeleteButton->setEnabled(true);
        m_sessionLoadButton->setEnabled(true);

        //
        // Get the session data
        SessionData session = m_sessions[sessionName];

        // Populate the session form
        populateSessionForm(session);

        // Store the current session name for later use
        m_currentSessionName = sessionName;

        // Highlight the selected card and unhighlight others
        for (const QString &name : m_sessionCards.keys()) {
            if (m_sessionCards[name]) {
                highlightCard(m_sessionCards[name], name == sessionName);
            }
        }
    }
}

QToolBar* MainWindow::createToolbar() {
    toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    // Web navigation actions - always visible
    m_webBackAction = toolbar->addAction(QIcon(":/resources/icons/arrow-left.svg"), "Back");
    m_webForwardAction = toolbar->addAction(QIcon(":/resources/icons/arrow-right.svg"), "Forward");
    m_webReloadAction = toolbar->addAction(QIcon(":/resources/icons/refresh-cw.svg"), "Reload");
    m_goHomeAction = toolbar->addAction(QIcon(":/resources/icons/home.svg"), "Home");
    // Add separator
    toolbar->addSeparator();

    // Add session and website actions - always visible
    //m_addCurrentSessionAction = toolbar->addAction(QIcon(":/resources/icons/save.svg"), "Save Current Session");
    m_addWebsiteFromUrlAction = toolbar->addAction(QIcon(":/resources/icons/file-plus.svg"), "Create Website from Current Url");
    //m_addCurrentWebsiteAction = toolbar->addAction(QIcon(":/resources/icons/plus.svg"), "Open Currently Selected Website");

    // Add separator
    //toolbar->addSeparator();

    // Add zoom actions - always visible
    m_zoomInAction = toolbar->addAction(QIcon(":/resources/icons/zoom-in.svg"), "Zoom In");
    m_zoomInAction->setVisible(false);
    m_zoomOutAction = toolbar->addAction(QIcon(":/resources/icons/zoom-out.svg"), "Zoom Out");
    m_zoomOutAction->setVisible(false);

    //toolbar->addSeparator();

    // Add copy URL action - always visible
    m_copyUrlAction = toolbar->addAction(QIcon(":/resources/icons/copy.svg"), "Copy Tab URL");

    // Add separator
    //toolbar->addSeparator();

    m_addCurrentWebsiteAction = toolbar->addAction(QIcon(":/resources/icons/plus.svg"), "Open Currently Selected Website");

    m_addCurrentSessionAction = toolbar->addAction(QIcon(":/resources/icons/save.svg"), "Save Current Session");

    //toolbar->addSeparator();
    //m_toggleViewAction = new QAction(QIcon(":/resources/icons/monitor.svg"), "Dashboard");
    //m_toggleViewAction = new QAction(QIcon(":/resources/icons/chevron-right.svg"), "Dashboard");
    m_toggleViewAction = new QAction(QIcon(":/resources/icons/repeat.svg"), "Dashboard");

    m_toggleViewAction->setToolTip("Switch between Dashboard and Web View");

    // Create a custom tool button that will display both icon and text
    toggleButton = new QToolButton();
    toggleButton->setDefaultAction(m_toggleViewAction);
    toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);



    // Add the button to the toolbar
    toolbar->addWidget(toggleButton);

    //toolbar->addSeparator();

    // Add tab count label
    m_tabCountLabel = new QLabel(":0", this);
    m_tabCountLabel->setVisible(false);
    m_tabCountLabel->setAlignment(Qt::AlignCenter);
    m_tabCountLabel->setMinimumWidth(80);
    toolbar->addWidget(m_tabCountLabel);

    toolbar->addSeparator();

    //close tabs
    // Add close all tabs button - always visible
    m_closeAllTabsAction = toolbar->addAction(QIcon(":/resources/icons/x.svg"), "Close All Tabs");
    m_closeAllTabsAction->setToolTip("Close All Open Tabs");

    //seperateprofiles toggle
    toolbar->addSeparator();
    QLabel *profileLabel = new QLabel("Private:");
    toolbar->addWidget(profileLabel);

    // Create modern toggle switch
    m_separateProfilesToggle = new QPushButton();
    m_separateProfilesToggle->setCheckable(true);
    m_separateProfilesToggle->setFixedSize(50, 25);

    m_separateProfilesToggle->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #ccc;"
        "    border-radius: 12px;"
        //"    background-color: #8e8e93;"
        "    background-color: #b0b0b0;"        // Pure gray (equal RGB values)
        "    text-align: center;"
        "    color: #ffffff;"
        "    padding: 4px 8px;"        // Reduced padding
        "    font-size: 12px;"         // Smaller font if needed
        "}"
        "QPushButton:checked {"
        "    background-color: #4CAF50;"
        "    border-color: #4CAF50;"
        "    color: #ffffff;"
        "}"
        );

    m_separateProfilesToggle->setText("OFF");
    m_separateProfilesToggle->setToolTip("Use Private profile when launching tabs");
    toolbar->addWidget(m_separateProfilesToggle);

    // Create a placeholder profile selector
    profileSelector = new QComboBox(this);
    profileSelector->setMaximumWidth(100);
    profileSelector->addItem("Loading...");
    profileSelector->setEnabled(false); // Disable it until it's populated
    toolbar->addWidget(profileSelector);


    toolbar->addSeparator();

    //theme togle
    // Add theme toggle switch
    m_themeToggle = new QPushButton();
    m_themeToggle->setCheckable(true);
    m_themeToggle->setFixedSize(50, 25);

    m_themeToggle->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #ccc;"
        "    border-radius: 12px;"
        "    background-color: #f0f0f0;"
        "    text-align: center;"
        "    font-size: 10px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #333;"
        "    border-color: #333;"
        "    color: white;"
        "}"
        );
    //m_themeToggle->setText("☀️");
    m_themeToggle->setIcon(QIcon(":/resources/icons/sun.svg"));

    m_themeToggle->setToolTip("Toggle Dark/Light Theme");
    toolbar->addWidget(m_themeToggle);

    // Connect theme toggle

    connect(m_themeToggle, &QPushButton::toggled, [this](bool dark) {

        if (dark) {
            m_isDarkTheme = true;
            setStyleSheet(loadDarkTheme());
            m_themeToggle->setIcon(QIcon(":/resources/icons-white/moon.svg"));
        } else {
            m_isDarkTheme = false;
            setStyleSheet(loadLightTheme());
            m_themeToggle->setIcon(QIcon(":/resources/icons/sun.svg"));
        }
    if (m_twoFAManager) {
        m_twoFAManager->setStyleSheet(styleSheet());
    }
    });

    //toolbar->addSeparator();
    m_downloadsAction = toolbar->addAction(QIcon(":/resources/icons/download.svg"), "Downloads");
    m_downloadsAction->setToolTip("Show Downloads");
    m_originalDownloadIcon = m_downloadsAction->icon(); // Store current icon

    //screenshot
    //toolbar->addSeparator();
    m_screenshotAction = toolbar->addAction(QIcon(":/resources/icons/camera.svg"), "Screenshot");
    m_screenshotAction->setToolTip("Take Screenshot of Current Tab");

    // Connect screenshot action
    connect(m_screenshotAction, &QAction::triggered, this, &MainWindow::takeScreenshot);

    //open downloads location
    //toolbar->addSeparator();
    m_openDownloadsFolderAction = toolbar->addAction(QIcon(":/resources/icons/folder.svg"), "Open Downloads Folder");
    m_openDownloadsFolderAction->setToolTip("Open Downloads Folder");
    m_openDownloadsFolderAction->setVisible(false);
    //2FA
    //toolbar->addSeparator();
    m_open2faManagerAction = toolbar->addAction(QIcon(":/resources/icons/shield.svg"), "2FAManager");
    m_open2faManagerAction->setToolTip("Open 2FA Manager");
    connect(m_open2faManagerAction, &QAction::triggered, this, &MainWindow::on_Open2faManager);

    // Open Copied Link in New Tab action
    //toolbar->addSeparator();
    m_openCopiedLinkAction = toolbar->addAction(QIcon(":/resources/icons/link.svg"), "Open Copied Link in New Tab");
    m_openCopiedLinkAction->setVisible(false);
    m_openCopiedLinkAction->setToolTip("Open copied link in a new tab - F11");
    m_openCopiedLinkAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_openCopiedLinkAction->setShortcutContext(Qt::ApplicationShortcut);
    // Connect the action
    connect(m_openCopiedLinkAction, &QAction::triggered, [this]() {
        QString clipboardText = QGuiApplication::clipboard()->text().trimmed();
        QRegularExpression urlRegex(R"(^(https?|ftp)://[^\s/$.?#].[^\s]*$)", QRegularExpression::CaseInsensitiveOption);

        if (urlRegex.match(clipboardText).hasMatch()) {
            createNewTabWithUrl(clipboardText);
        } else {
            QMessageBox::warning(this, tr("Invalid Link"), tr("Clipboard does not contain a valid link."));
        }
    });

    // URLBar toggle action
    //toolbar->addSeparator();
    m_toggleUrlBarAction = toolbar->addAction(QIcon(":/resources/icons/eye.svg"), "Show URL Bar");
    m_toggleUrlBarAction->setToolTip("Toggle URL Bar Visibility");

    // Connect the toggle
    connect(m_toggleUrlBarAction, &QAction::triggered, [this]() {
        QWidget* current = m_stackedWidget->currentWidget();

        if (m_stackedWidget->currentWidget() != m_webViewContainer) {

            QMessageBox::information(this, "URL Bar Toggle",
                                     "The URL bar is only visible in Web View mode.");
            return;
        }

        m_urlBar->setVisible(!m_urlBar->isVisible());
        updateUrlBarState(); // Update state after toggle
    });
    // end urlbar

    connect(m_openDownloadsFolderAction, &QAction::triggered, this, &MainWindow::openDownloadsFolder);



    connect(m_separateProfilesToggle, &QPushButton::toggled, [this](bool checked) {
        m_usingSeparateProfiles = checked;
        m_separateProfilesToggle->setText(checked ? "ON" : "OFF");
        m_separateProfilesToggle->setToolTip(checked ? "Using private profile" : "Using shared profile");

        // Disable the profile selector if separate profiles are enabled
        if (profileSelector) {
            profileSelector->setEnabled(!checked);

            // Add a tooltip explaining why it's disabled
            if (checked) {
                profileSelector->setToolTip("Named profiles are disabled when separate profiles are enabled");

                // Reset to default profile if separate profiles are enabled
                profileSelector->setCurrentIndex(0); // Assuming "Default" is at index 0
                m_usingNamedProfiles = false;
                m_selectedProfileName = "";
                saveNamedProfilesData();
            } else {
                profileSelector->setToolTip("Select a profile to use");
            }
        }
    });



    // Connect close all tabs action
    connect(m_closeAllTabsAction, &QAction::triggered, [this]() {

        int tabCount = m_tabWidget->count();
            if (tabCount == 0) return;

            QSettings settings;
            bool dontShowAgain = settings.value("dontShowCloseAllWarning", false).toBool();

            if (!dontShowAgain) {
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Close All Tabs");
                msgBox.setText(QString("You are about to close %1 tab(s). Do you want to proceed?").arg(tabCount));
                msgBox.setIcon(QMessageBox::Question);

                // Create a checkbox
                QCheckBox* dontShowCheckBox = new QCheckBox("Don't show this again");

                // Add checkbox to message box (using layout)
                msgBox.setCheckBox(dontShowCheckBox);

                // Add buttons
                QPushButton* proceedButton = msgBox.addButton("Proceed", QMessageBox::AcceptRole);
                QPushButton* cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
                msgBox.setDefaultButton(cancelButton);

                // Show message box
                msgBox.exec();

                // Save checkbox state
                if (dontShowCheckBox->isChecked()) {
                    settings.setValue("dontShowCloseAllWarning", true);
                }

                // If user clicked Cancel, return
                if (msgBox.clickedButton() != proceedButton) {
                    return;
                }
            }

        while (m_tabWidget->count() > 0) {
            onTabCloseRequested(0);
        }
    });

    // Connect downloads action
    connect(m_downloadsAction, &QAction::triggered, [this]() {
        downloadManager->showDownloadsWindow();
    });

    // Connect web navigation actions
    connect(m_webBackAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->back();
    });
    connect(m_webForwardAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->forward();
    });
    connect(m_webReloadAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->reload();
    });

    // Connect new actions
    //connect(m_addCurrentSessionAction, &QAction::triggered, this, &MainWindow::onSaveCurrentSession);
    connect(m_addCurrentSessionAction, &QAction::triggered, this, &MainWindow::onSaveSession);

    connect(m_addCurrentWebsiteAction, &QAction::triggered, this, &MainWindow::onLaunchWebsite);
    connect(m_zoomInAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            view->setZoomFactor(view->zoomFactor() + 0.1);
        }
    });
    connect(m_zoomOutAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            view->setZoomFactor(view->zoomFactor() - 0.1);
        }
    });
    connect(m_copyUrlAction, &QAction::triggered, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            QUrl url = view->url();
            if (url.isValid()) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(url.toString());
                statusBar()->showMessage("URL copied to clipboard", 3000);
            }
        }
    });

    // Connect toggle view action
    connect(m_toggleViewAction, &QAction::triggered, this, &MainWindow::toggleView);

    connect(m_goHomeAction, &QAction::triggered, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            // Get the initial URL from the web view's history
            QWebEngineHistory *history = view->history();
            if (history->count() > 0) {
                // Go to the first item in history (the initial URL)
                QWebEngineHistoryItem firstItem = history->itemAt(0);
                view->load(firstItem.url());
            }
        }
    });

    connect(m_addWebsiteFromUrlAction, &QAction::triggered, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            QString currentUrl = view->url().toString();
            if (currentUrl.isEmpty() || currentUrl == "about:blank") {
                QMessageBox::information(this, "No URL", "No valid URL to add as website.");
                return;
            }

            bool ok;
            QString title = QInputDialog::getText(this, "Add Website",
                                                  "Enter title for this website:",
                                                  QLineEdit::Normal,
                                                  "", &ok);
            if (ok && !title.isEmpty()) {
                // Check if website already exists
                for (int i = 0; i < m_model->rowCount(); ++i) {
                    Website existing = m_model->getWebsite(i);
                    if (existing.url == currentUrl) {
                        QMessageBox::information(this, "Already Exists", "This website is already in your list.");
                        return;
                    }
                }

                // Create new website
                Website newWebsite;
                newWebsite.url = currentUrl;
                newWebsite.title = title;
                newWebsite.favicon = view->icon();
                newWebsite.lastVisited = QDateTime::currentDateTime();
                newWebsite.visitCount = 1;

                m_model->addWebsite(newWebsite);
                updateWebsiteCards();
                statusBar()->showMessage("Website added: " + title, 3000);
            }
        }
    });
    //toolbar->addSeparator();
    //search
    streamButton = new QPushButton(this);
    streamButton->setIcon(QIcon(":/resources/icons/rss.svg"));
    streamButton->setToolTip("Open Ad-Free Player\n\n"
                             "1. Click to open the ad-free video player dialog\n"
                             "2. In the player, click 'Get URL' to fetch the current URL from the address bar\n"
                             "3. Works with YouTube, Vimeo, Dailymotion, Rumble, and Odysee\n"
                             "4. Plays videos without any ads - yt-dlp extracts the direct stream URL");
    streamButton->setCheckable(true);
    streamButton->setChecked(false);

    connect(streamButton, &QPushButton::clicked, this, [this](bool checked){
        if(checked){
            player->show();
            player->raise();
        }else{
            player->hide();
        }
    });
    toolbar->addWidget(streamButton);

    searchAction = new QAction(this);
    searchAction->setCheckable(true);
    searchAction->setChecked(false);
    searchAction->setIcon(QIcon(":/resources/icons/search.svg"));
    searchAction->setToolTip("Enable/Disable the search field");
    connect(searchAction, &QAction::toggled, [this](bool checked) {

        m_searchLineEdit->setEnabled(checked);

    });

    // Create hidden line edit
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setObjectName("searchLineEdit");
    m_searchLineEdit->setMaximumWidth(80);

    m_searchLineEdit->setPlaceholderText("Search");
    m_searchLineEdit->setToolTip("Search Cards");

    m_searchLineEdit->setEnabled(false);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::searchCards);

    // Add to toolbar
    toolbar->addAction(searchAction);
    toolbar->addWidget(m_searchLineEdit);


    highlightButtons(buttonsHighlighted);

    return toolbar;
}

void MainWindow::toggleView() {
    if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
        // Currently in dashboard, switch to web view
        m_stackedWidget->setCurrentWidget(m_webViewContainer);
        /*
        if (m_isDarkTheme) {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/chevron-left.svg"));
        } else {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-left.svg"));
        }
        */
        m_toggleViewAction->setToolTip("Switch to Dashboard");
        setWindowTitle("Jasmine - Web Browser");

        // Allow resizing in web view
        this->setMinimumSize(0, 0);
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        // Restore previous web view size if available, otherwise use dashboard size
        if (!m_savedWebViewSize.isEmpty()) {
            this->resize(m_savedWebViewSize);
        } else {
            this->resize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);
        }

        m_stackedWidget->update();

    } else {
        // Currently in web view, switch to dashboard
        m_savedWebViewSize = this->size();
        m_stackedWidget->setCurrentWidget(m_dashboardWidget);
        if (this->isMaximized()) {
            this->setWindowState(Qt::WindowNoState);
        }

        // Lock dashboard size - prevents all resizing including drag
        this->setFixedSize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);
        /*
        if (m_isDarkTheme) {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/chevron-right.svg"));
        } else {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-right.svg"));
        }
        */

        m_toggleViewAction->setToolTip("Switch to Webview");
        setWindowTitle("Jasmine");

        m_urlBar->setVisible(false);

        m_stackedWidget->update();
    }

    updateUrlBarState();
    m_toggleViewAction->setEnabled(true);
    updateTabCountStatus();
}


void MainWindow::onAddCurrentWebsite()
{

}

void MainWindow::updateTabCountStatus() {
    int tabCount = m_tabWidget->count();
    // Update the tab count label
    //m_tabCountLabel->setText(QString("Tabs: %1").arg(tabCount));
    m_toggleViewAction->setText(QString("Tabs: %1").arg(tabCount));
    // Update status bar with tab count
    if (tabCount > 0) {
        statusBar()->showMessage(QString("%1 tab%2 open").arg(tabCount).arg(tabCount > 1 ? "s" : ""), 3000);
    } else {
        statusBar()->showMessage("No tabs open", 3000);
    }

    // ALWAYS keep the toggle button enabled
    m_toggleViewAction->setEnabled(true);

}


void MainWindow::selectFirstItemIfNoneSelected() {
    // For websites tab
    if (m_leftPanelTabs->currentIndex() == 0) {
        if (m_currentWebsiteIndex == -1 && m_model->rowCount() > 0) {
            // Select the first website
            QModelIndex firstIndex = m_model->index(0, 0);
            onWebsiteSelected(firstIndex);
        } else if (m_currentWebsiteIndex >= 0 && m_currentWebsiteIndex < m_model->rowCount()) {
            // Reselect the previously selected website
            QModelIndex index = m_model->index(m_currentWebsiteIndex, 0);
            onWebsiteSelected(index);
        }
    }
    // For sessions tab
    else if (m_leftPanelTabs->currentIndex() == 1) {
        if (m_currentSessionName.isEmpty() && !m_sessions.isEmpty()) {
            // Select the first session
            QString firstName = m_sessions.keys().first();
            onSessionSelected(firstName);
        } else if (!m_currentSessionName.isEmpty() && m_sessions.contains(m_currentSessionName)) {
            // Reselect the previously selected session
            onSessionSelected(m_currentSessionName);
        }
    }
}


QIcon MainWindow::generateRandomSvgIcon() {
    // Random number generator
    QRandomGenerator *rng = QRandomGenerator::global();

    // Choose a color scheme (analogous colors)
    int baseHue = rng->bounded(360);
    QColor color1, color2, color3;

    color1.setHsv(baseHue, 180 + rng->bounded(75), 180 + rng->bounded(75));
    color2.setHsv((baseHue + 30) % 360, 180 + rng->bounded(75), 180 + rng->bounded(75));
    color3.setHsv((baseHue + 60) % 360, 180 + rng->bounded(75), 180 + rng->bounded(75));

    // Choose a pattern type
    int patternType = rng->bounded(5);

    QString svgContent = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\">";

    // Background
    svgContent += QString("<rect width=\"64\" height=\"64\" fill=\"%1\" />").arg(color1.name());

    switch (patternType) {


    case 0: {
        // Gradient with shapes
        svgContent += QString(
                          "<defs>"
                          "  <linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">"
                          "    <stop offset=\"0%\" style=\"stop-color:%1;stop-opacity:1\" />"
                          "    <stop offset=\"100%\" style=\"stop-color:%2;stop-opacity:1\" />"
                          "  </linearGradient>"
                          "</defs>"
                          "<rect width=\"64\" height=\"64\" fill=\"url(#grad)\" />"
                          ).arg(color1.name()).arg(color2.name());

        // Add some shapes
        int numShapes = 3 + rng->bounded(5);
        for (int i = 0; i < numShapes; i++) {
            int x = rng->bounded(64);
            int y = rng->bounded(64);
            int size = 10 + rng->bounded(20);

            if (rng->bounded(2) == 0) {
                // Rectangle
                svgContent += QString("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%3\" "
                                      "fill=\"%4\" opacity=\"0.7\" transform=\"rotate(%5,%1,%2)\" />")
                                  .arg(x).arg(y).arg(size).arg(color3.name())
                                  .arg(rng->bounded(360));
            } else {
                // Triangle
                int x2 = x + size;
                int y2 = y;
                int x3 = x + (size/2);
                int y3 = y + size;

                svgContent += QString("<polygon points=\"%1,%2 %3,%4 %5,%6\" "
                                      "fill=\"%7\" opacity=\"0.7\" />")
                                  .arg(x).arg(y).arg(x2).arg(y2).arg(x3).arg(y3).arg(color3.name());
            }
        }
        break;
    }

    case 1: {
        // Wavy pattern - horizontal OR vertical
        bool isVertical = rng->bounded(2) == 0;

        if (isVertical) {
            // Vertical waves
            svgContent += QString(
                              "<defs>"
                              "  <pattern id=\"pattern1\" patternUnits=\"userSpaceOnUse\" width=\"20\" height=\"20\">"
                              "    <path d=\"M10,0 Q20,5 10,10 T10,20\" stroke=\"%1\" fill=\"none\" stroke-width=\"2\" />"
                              "  </pattern>"
                              "  <pattern id=\"pattern2\" patternUnits=\"userSpaceOnUse\" width=\"20\" height=\"20\">"
                              "    <path d=\"M5,0 Q15,5 5,10 T5,20\" stroke=\"%2\" fill=\"none\" stroke-width=\"2\" />"
                              "  </pattern>"
                              "</defs>"
                              ).arg(color2.name()).arg(color3.name());
        } else {
            // Horizontal waves (your original)
            svgContent += QString(
                              "<defs>"
                              "  <pattern id=\"pattern1\" patternUnits=\"userSpaceOnUse\" width=\"20\" height=\"20\">"
                              "    <path d=\"M0,10 Q5,20 10,10 T20,10\" stroke=\"%1\" fill=\"none\" stroke-width=\"2\" />"
                              "  </pattern>"
                              "  <pattern id=\"pattern2\" patternUnits=\"userSpaceOnUse\" width=\"20\" height=\"20\">"
                              "    <path d=\"M0,5 Q5,15 10,5 T20,5\" stroke=\"%2\" fill=\"none\" stroke-width=\"2\" />"
                              "  </pattern>"
                              "</defs>"
                              ).arg(color2.name()).arg(color3.name());
        }

        // Apply patterns (same for both)
        svgContent += QString(
                          "<rect width=\"64\" height=\"64\" fill=\"%1\" />"
                          "<rect width=\"64\" height=\"64\" fill=\"url(#pattern1)\" opacity=\"0.7\" />"
                          "<rect width=\"64\" height=\"64\" fill=\"url(#pattern2)\" opacity=\"0.5\" />"
                          ).arg(color1.name());
        break;
    }


    case 2: {
        // Geometric mosaic
        int gridSize = 4 + rng->bounded(4); // 4x4 to 7x7 grid
        int cellSize = 64 / gridSize;

        for (int x = 0; x < gridSize; x++) {
            for (int y = 0; y < gridSize; y++) {
                int shapeType = rng->bounded(3);
                QColor cellColor;

                // Choose color based on position
                int colorChoice = (x + y) % 3;
                if (colorChoice == 0) cellColor = color1;
                else if (colorChoice == 1) cellColor = color2;
                else cellColor = color3;

                // Adjust brightness randomly
                int h, s, v;
                cellColor.getHsv(&h, &s, &v);
                v = qMin(255, v + rng->bounded(-30, 30));
                cellColor.setHsv(h, s, v);

                int xPos = x * cellSize;
                int yPos = y * cellSize;

                if (shapeType == 0) {
                    // Square
                    svgContent += QString("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%3\" fill=\"%4\" />")
                                      .arg(xPos).arg(yPos).arg(cellSize).arg(cellColor.name());
                } else if (shapeType == 1) {
                    // Circle
                    svgContent += QString("<circle cx=\"%1\" cy=\"%2\" r=\"%3\" fill=\"%4\" />")
                                      .arg(xPos + cellSize/2).arg(yPos + cellSize/2)
                                      .arg(cellSize/2).arg(cellColor.name());
                } else {
                    // Triangle
                    if (rng->bounded(2) == 0) {
                        // Top-left to bottom-right diagonal
                        svgContent += QString("<polygon points=\"%1,%2 %3,%4 %5,%6\" fill=\"%7\" />")
                                          .arg(xPos).arg(yPos)
                                          .arg(xPos + cellSize).arg(yPos + cellSize)
                                          .arg(xPos).arg(yPos + cellSize)
                                          .arg(cellColor.name());
                    } else {
                        // Top-right to bottom-left diagonal
                        svgContent += QString("<polygon points=\"%1,%2 %3,%4 %5,%6\" fill=\"%7\" />")
                                          .arg(xPos + cellSize).arg(yPos)
                                          .arg(xPos).arg(yPos + cellSize)
                                          .arg(xPos + cellSize).arg(yPos + cellSize)
                                          .arg(cellColor.name());
                    }
                }
            }
        }
        break;
    }

    case 3: {
        // Abstract art
        // Background gradient
        svgContent += QString(
                          "<defs>"
                          "  <radialGradient id=\"grad\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"50%\" fy=\"50%\">"
                          "    <stop offset=\"0%\" style=\"stop-color:%1;stop-opacity:1\" />"
                          "    <stop offset=\"100%\" style=\"stop-color:%2;stop-opacity:1\" />"
                          "  </radialGradient>"
                          "</defs>"
                          "<rect width=\"64\" height=\"64\" fill=\"url(#grad)\" />"
                          ).arg(color1.name()).arg(color2.name());

        // Add random bezier curves
        int numCurves = 3 + rng->bounded(5);
        for (int i = 0; i < numCurves; i++) {
            int x1 = rng->bounded(64);
            int y1 = rng->bounded(64);
            int x2 = rng->bounded(64);
            int y2 = rng->bounded(64);
            int cx1 = rng->bounded(64);
            int cy1 = rng->bounded(64);
            int cx2 = rng->bounded(64);
            int cy2 = rng->bounded(64);

            svgContent += QString("<path d=\"M%1,%2 C%3,%4 %5,%6 %7,%8\" stroke=\"%9\" "
                                  "stroke-width=\"%10\" fill=\"none\" opacity=\"0.7\" />")
                              .arg(x1).arg(y1).arg(cx1).arg(cy1).arg(cx2).arg(cy2).arg(x2).arg(y2)
                              .arg(color3.name()).arg(2 + rng->bounded(4));
        }
        break;
    }
    case 4: {
        // Mandala pattern
        int centerX = 32, centerY = 32;
        int layers = 3 + rng->bounded(3); // 3-5 layers

        for (int layer = 0; layer < layers; layer++) {
            int radius = 8 + (layer * 8); // Expanding rings
            int petals = 6 + (layer * 2); // More petals in outer rings
            QColor layerColor = (layer % 3 == 0) ? color1 :
                                    (layer % 3 == 1) ? color2 : color3;

            for (int i = 0; i < petals; i++) {
                double angle = (2 * M_PI * i) / petals;
                int x = centerX + radius * cos(angle);
                int y = centerY + radius * sin(angle);

                // Draw petal shapes
                if (layer % 2 == 0) {
                    // Circles
                    svgContent += QString("<circle cx=\"%1\" cy=\"%2\" r=\"%3\" fill=\"%4\" opacity=\"0.8\" />")
                                      .arg(x).arg(y).arg(3 + layer).arg(layerColor.name());
                } else {
                    // Diamond shapes
                    svgContent += QString("<polygon points=\"%1,%2 %3,%4 %5,%6 %7,%8\" fill=\"%9\" opacity=\"0.7\" />")
                                      .arg(x).arg(y-4).arg(x+4).arg(y).arg(x).arg(y+4).arg(x-4).arg(y)
                                      .arg(layerColor.name());
                }
            }
        }

        // Center circle
        svgContent += QString("<circle cx=\"32\" cy=\"32\" r=\"6\" fill=\"%1\" />").arg(color2.name());
        break;
        }
    }
    svgContent += "</svg>";

    // Convert SVG to icon
    QSvgRenderer renderer(svgContent.toUtf8());
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return QIcon(pixmap);
}



void MainWindow::onUpdateSession() {
    if (m_currentSessionName.isEmpty()) {
        return;
    }

    // Check if name changed and warn about new icon
    QString newName = m_sessionNameInput->text().trimmed();
    if (newName.isEmpty()) {
        QMessageBox::information(this, "Session Update",
                                 "Please set the Session name first!");
        return;
    }

    // Update the session data
    m_sessions[m_currentSessionName].comments = m_sessionCommentsEdit->toPlainText();

    // If name changed, handle the map key change
    if (newName != m_currentSessionName && !newName.isEmpty()) {
        SessionData sessionData = m_sessions[m_currentSessionName];
        sessionData.name = newName;
        m_sessions.remove(m_currentSessionName);
        m_sessions[newName] = sessionData;
        m_currentSessionName = newName;
    }

    // Save the current icon to disk
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    QDir().mkpath(iconDir);
    QString iconPath = iconDir + m_currentSessionName + "_session.png";
    QPixmap pixmap = m_sessions[m_currentSessionName].icon.pixmap(64, 64);
    pixmap.save(iconPath, "PNG");

    // Save to disk
    saveSessionsData();

    // Update UI
    updateSessionCards();

    // Success message
    QMessageBox::information(this, "Success", "Session successfully updated!");
}


void MainWindow::takeScreenshot(){
    QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget());
    if (!currentView) {
        return;
    }
    QPixmap screenshot = currentView->grab();
    if (screenshot.isNull()) {
        return;
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("screenshot_%1.png").arg(timestamp);



    QDir screenshotDir(JASMINE_CONSTANTS::screenshotsDirPath);
    if (!screenshotDir.exists()) {
        screenshotDir.mkpath(".");
    }

    QString filepath = screenshotDir.filePath(filename);
    if (screenshot.save(filepath)) {
    #ifdef FLATPAK_BUILD
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Screenshot");
            msgBox.setText("Screenshot saved: " + filename);
            msgBox.setInformativeText("Location: " + screenshotDir.absolutePath()); // Fixed: was downloadsDir
            QPushButton *copyButton = msgBox.addButton("Copy Path", QMessageBox::ActionRole);
            msgBox.addButton(QMessageBox::Ok);
            msgBox.exec();

            if (msgBox.clickedButton() == copyButton) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(screenshotDir.absolutePath()); // Fixed: was downloadsDir
            }
    #else
            QMessageBox::information(this, "Screenshot", "Screenshot saved in " + screenshotDir.absolutePath() + " as "  + filename);
    #endif
        } else {
            QMessageBox::warning(this, "Error", "Failed to save screenshot.");
        }
    }



void MainWindow::openDownloadsFolder(){



    QDir downloadsDir(JASMINE_CONSTANTS::downloadsDirPath);

    // Create directory if it doesn't exist
    if (!downloadsDir.exists()) {
        downloadsDir.mkpath(".");
    }

#ifdef FLATPAK_BUILD
    // Show helpful message for Flatpak users
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Downloads Location");
    msgBox.setText(QString("Your downloads are saved to:\n\n%1\n\n"
                           "You can access this folder using your system's file manager.")
                       .arg(JASMINE_CONSTANTS::downloadsDirPath));
    msgBox.setStandardButtons(QMessageBox::Ok);
    QPushButton *copyButton = msgBox.addButton("Copy Path", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QApplication::clipboard()->setText(JASMINE_CONSTANTS::downloadsDirPath);
    }
#else
    // Open in system file manager
    QDesktopServices::openUrl(QUrl::fromLocalFile(JASMINE_CONSTANTS::downloadsDirPath));
#endif
}

QString MainWindow::loadDarkTheme(){
    QFile styleFile(":/resources/dark-theme.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString stylesheet = stream.readAll();
        styleFile.close();

        // Switch to white icons for dark theme
        m_webBackAction->setIcon(QIcon(":/resources/icons-white/arrow-left.svg"));
        m_webForwardAction->setIcon(QIcon(":/resources/icons-white/arrow-right.svg"));
        m_webReloadAction->setIcon(QIcon(":/resources/icons-white/refresh-cw.svg"));
        m_addCurrentSessionAction->setIcon(QIcon(":/resources/icons-white/save.svg"));
        m_addCurrentWebsiteAction->setIcon(QIcon(":/resources/icons-white/plus.svg"));
        m_zoomInAction->setIcon(QIcon(":/resources/icons-white/zoom-in.svg"));
        m_zoomOutAction->setIcon(QIcon(":/resources/icons-white/zoom-out.svg"));
        m_copyUrlAction->setIcon(QIcon(":/resources/icons-white/copy.svg"));
        m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/repeat.svg"));
        m_openCopiedLinkAction->setIcon(QIcon(":/resources/icons-white/link.svg"));
        streamButton->setIcon(QIcon(":/resources/icons-white/rss.svg"));
        searchAction->setIcon(QIcon(":/resources/icons-white/search.svg"));

        /*
        if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/chevron-right.svg"));
        } else {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/chevron-left.svg"));
        }
        */

        m_downloadsAction->setIcon(QIcon(":/resources/icons-white/download.svg"));
        m_closeAllTabsAction->setIcon(QIcon(":/resources/icons-white/x.svg"));
        m_screenshotAction->setIcon(QIcon(":/resources/icons-white/camera.svg"));
        m_openDownloadsFolderAction->setIcon(QIcon(":/resources/icons-white/folder.svg"));
        m_leftPanelTabs->setTabIcon(0, createRotatedIcon(":/resources/icons-white/globe.svg"));
        m_leftPanelTabs->setTabIcon(1, createRotatedIcon(":/resources/icons-white/bookmark.svg"));
        m_leftPanelTabs->setTabIcon(2, createRotatedIcon(":/resources/icons-white/radio.svg"));
        m_leftPanelTabs->setTabIcon(3, createRotatedIcon(":/resources/icons-white/tv.svg"));
        m_leftPanelTabs->setTabIcon(4, createRotatedIcon(":/resources/icons-white/rss.svg"));

        m_open2faManagerAction->setIcon(QIcon(":/resources/icons-white/shield.svg"));
        m_usernameEyeButton->setIcon(QIcon(":/resources/icons-white/eye.svg"));
        m_passwordEyeButton->setIcon(QIcon(":/resources/icons-white/eye.svg"));
        m_toggleUrlBarAction->setIcon(QIcon(":/resources/icons-white/eye.svg"));
        if (m_urlBar) {
            m_urlBar->updateTheme(true);
        }
        m_addWebsiteFromUrlAction->setIcon(QIcon(":/resources/icons-white/file-plus.svg"));
        m_goHomeAction->setIcon(QIcon(":/resources/icons-white/home.svg"));

        highlightButtons(buttonsHighlighted);

        return stylesheet;
    }

    return QString();
}



QString MainWindow::loadLightTheme(){
    QFile styleFile(":/resources/stylesheet.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString stylesheet = stream.readAll();
        styleFile.close();

        // Switch to black icons for light theme
        m_webBackAction->setIcon(QIcon(":/resources/icons/arrow-left.svg"));
        m_webForwardAction->setIcon(QIcon(":/resources/icons/arrow-right.svg"));
        m_webReloadAction->setIcon(QIcon(":/resources/icons/refresh-cw.svg"));
        m_addCurrentSessionAction->setIcon(QIcon(":/resources/icons/save.svg"));
        m_addCurrentWebsiteAction->setIcon(QIcon(":/resources/icons/plus.svg"));
        m_zoomInAction->setIcon(QIcon(":/resources/icons/zoom-in.svg"));
        m_zoomOutAction->setIcon(QIcon(":/resources/icons/zoom-out.svg"));
        m_copyUrlAction->setIcon(QIcon(":/resources/icons/copy.svg"));
        m_toggleViewAction->setIcon(QIcon(":/resources/icons/repeat.svg"));
        m_openCopiedLinkAction->setIcon(QIcon(":/resources/icons/link.svg"));
        streamButton->setIcon(QIcon(":/resources/icons/rss.svg"));
        searchAction->setIcon(QIcon(":/resources/icons/search.svg"));

        /*
        if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-right.svg"));
        } else {
            m_toggleViewAction->setIcon(QIcon(":/resources/icons/chevron-left.svg"));
        }
        */
        m_downloadsAction->setIcon(QIcon(":/resources/icons/download.svg"));
        m_closeAllTabsAction->setIcon(QIcon(":/resources/icons/x.svg"));
        m_screenshotAction->setIcon(QIcon(":/resources/icons/camera.svg"));
        m_openDownloadsFolderAction->setIcon(QIcon(":/resources/icons/folder.svg"));
        m_leftPanelTabs->setTabIcon(0, createRotatedIcon(":/resources/icons/globe.svg"));
        m_leftPanelTabs->setTabIcon(1, createRotatedIcon(":/resources/icons/bookmark.svg"));
        m_leftPanelTabs->setTabIcon(2, createRotatedIcon(":/resources/icons/radio.svg"));
        m_leftPanelTabs->setTabIcon(3, createRotatedIcon(":/resources/icons/tv.svg"));
        m_leftPanelTabs->setTabIcon(4, createRotatedIcon(":/resources/icons/rss.svg"));

        m_open2faManagerAction->setIcon(QIcon(":/resources/icons/shield.svg"));
        m_usernameEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));
        m_passwordEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));
        m_toggleUrlBarAction->setIcon(QIcon(":/resources/icons/eye.svg"));
        if (m_urlBar) {
            m_urlBar->updateTheme(false);
        }
        m_addWebsiteFromUrlAction->setIcon(QIcon(":/resources/icons/file-plus.svg"));
        m_goHomeAction->setIcon(QIcon(":/resources/icons/home.svg"));

        highlightButtons(buttonsHighlighted);

        return stylesheet;
    }
    return QString();
}

QIcon MainWindow::createRotatedIcon(const QString& iconPath, int degrees) {
    QPixmap pixmap(iconPath);
    QTransform transform;
    transform.rotate(degrees);
    QPixmap rotatedPixmap = pixmap.transformed(transform, Qt::SmoothTransformation);
    return QIcon(rotatedPixmap);
}


void MainWindow::onCleanSharedProfileData() {
    if (QMessageBox::question(this, "Clean Shared Profile Data",
                              "Are you sure you want to clean shared profile browsing data? This will affect tabs using the shared profile but not separate profiles.",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        // Clear only the shared profile
        m_webProfile->clearAllVisitedLinks();
        m_webProfile->clearHttpCache();
        m_webProfile->cookieStore()->deleteAllCookies();

        statusBar()->showMessage("Shared profile data has been cleared.", 3000);
    }
}

void MainWindow::onRestoreFactoryDefaults() {
    QMessageBox::StandardButton reply = QMessageBox::warning(this,
                                                             "Restore Factory Defaults",
                                                             "⚠️ WARNING: This will permanently delete:\n\n"
                                                             "• All browsing data (cookies, cache, history)\n"
                                                             "• All saved sessions\n"
                                                             "• All website bookmarks\n"
                                                             "• All application settings\n"
                                                             "• All profile data\n\n"
                                                             "This action CANNOT be undone!\n\n"
                                                             "Are you absolutely sure?",
                                                             QMessageBox::Yes | QMessageBox::No,
                                                             QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Final confirmation
        QMessageBox::StandardButton finalReply = QMessageBox::critical(this,
                                                                       "FINAL WARNING",
                                                                       "🚨 LAST CHANCE!\n\nThis will delete ALL your data permanently.\n\nProceed with factory reset?",
                                                                       QMessageBox::Yes | QMessageBox::No,
                                                                       QMessageBox::No);

        if (finalReply == QMessageBox::Yes) {
            performFactoryReset();
        }
    }
}

void MainWindow::on_Open2faManager()
{
    if (!m_twoFAManager) {
        m_twoFAManager = new TwoFAManager(this);
        // Apply current theme to the new window
        if (!styleSheet().isEmpty()) {  // If dark theme is active
            m_twoFAManager->setStyleSheet(styleSheet());
        }
        connect(m_twoFAManager, &QObject::destroyed, [this]() {
            m_twoFAManager = nullptr;
        });
    }

    m_twoFAManager->show();
    m_twoFAManager->raise();
    m_twoFAManager->activateWindow();
}


void MainWindow::performFactoryReset() {
    // 1. Clear the model
    m_model->setWebsites(QList<Website>());

    // 2. Clear sessions
    m_sessions.clear();

    // 3. Clear QSettings
    QSettings settings;
    settings.clear();
    settings.sync();
    //clean security manager
    m_securityManager->clearSecuritySettings();

    // 4. Clear browsing data
    onCleanAllData();

    // 5. Delete directories - use the already-set values!
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir(appDataDir).removeRecursively();

    // No need for orgName/appName - QStandardPaths uses the values from main.cpp!

    statusBar()->showMessage("Factory reset complete. Please restart the application.", 5000);
    QApplication::quit();
}


bool MainWindow::checkStartupSecurity() {
    return m_securityManager->checkPasswordOnStartup();
}


void MainWindow::updateUrlBarState() {
    bool inWebView = (m_stackedWidget->currentWidget() == m_webViewContainer);
    bool urlBarVisible = m_urlBar->isVisible();



    // Update tooltip based on current state
    if (inWebView) {
        m_toggleUrlBarAction->setToolTip(urlBarVisible ? "Hide URL Bar" : "Show URL Bar");
        m_toggleUrlBarAction->setText(urlBarVisible ? "Hide URL Bar" : "Show URL Bar"); // Added this line
    }
}

void MainWindow::connectUrlBar() {


    connect(m_urlBar, &URLBar::urlChanged, this, [this](const QString &url) {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            // Tab exists, load URL in current tab
            view->load(QUrl(url));
            //createNewTabWithUrlFromLink(url,view);

        } else {
            // No tabs open, create a new tab with this URL
            createNewTabWithUrl(url);
        }
    });

    connect(m_urlBar, &URLBar::backRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->back();
    });

    connect(m_urlBar, &URLBar::forwardRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->forward();
    });

    connect(m_urlBar, &URLBar::reloadRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget()))
            view->reload();
    });

    connect(m_urlBar, &URLBar::homeRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            // Get the initial URL from the web view's history
            QWebEngineHistory *history = view->history();
            if (history->count() > 0) {
                // Go to the first item in history (the initial URL)
                QWebEngineHistoryItem firstItem = history->itemAt(0);
                view->load(firstItem.url());
            }
        }
    });

    connect(m_urlBar, &URLBar::copyUrlRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            QUrl url = view->url();
            if (url.isValid()) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(url.toString());
                statusBar()->showMessage("URL copied to clipboard", 3000);
            }
        }
    });

    connect(m_urlBar, &URLBar::addWebsiteRequested, this, [this]() {
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            QString currentUrl = view->url().toString();
            if (currentUrl.isEmpty() || currentUrl == "about:blank") {
                QMessageBox::information(this, "No URL", "No valid URL to add as website.");
                return;
            }

            bool ok;
            QString title = QInputDialog::getText(this, "Add Website",
                                                  "Enter title for this website:",
                                                  QLineEdit::Normal,
                                                  "", &ok);
            if (ok && !title.isEmpty()) {
                // Check if website already exists
                for (int i = 0; i < m_model->rowCount(); ++i) {
                    Website existing = m_model->getWebsite(i);
                    if (existing.url == currentUrl) {
                        QMessageBox::information(this, "Already Exists", "This website is already in your list.");
                        return;
                    }
                }

                // Create new website
                Website newWebsite;
                newWebsite.url = currentUrl;
                newWebsite.title = title;
                newWebsite.favicon = view->icon();
                newWebsite.lastVisited = QDateTime::currentDateTime();
                newWebsite.visitCount = 1;

                m_model->addWebsite(newWebsite);
                updateWebsiteCards();
                statusBar()->showMessage("Website added: " + title, 3000);
            }
        }
    });

    connect(m_urlBar, &URLBar::newTabRequested, this, [this]() {
        createNewTab();

    });
}

void MainWindow::createNewTabWithUrl(const QString &url) {
    QWebEngineView *webView = new QWebEngineView(m_tabWidget);
    // URL sync connection
    connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
        if (QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            if (sender() == currentView) {
                m_urlBar->setUrl(url.toString());
            }
        }
    });
    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        int tabIndex = m_tabWidget->indexOf(webView);
        if (tabIndex != -1) {
            QString displayTitle = title.isEmpty() ? "New Tab" : title;
            if (displayTitle.length() > 20) {
                displayTitle = displayTitle.left(17) + "...";
            }
            m_tabWidget->setTabText(tabIndex, displayTitle);

        }
    });
    // Determine which profile to use
    QWebEngineProfile *profile;



    if (m_usingNamedProfiles && !m_selectedProfileName.isEmpty()) {
        profile = getOrCreateNamedProfile(m_selectedProfileName);
        m_tabNamedProfiles[webView] = m_selectedProfileName;
    } else if (m_usingSeparateProfiles) {
        profile = createProfileForTab();
        m_tabProfiles[webView] = profile;

    } else {
        profile = m_webProfile;
    }



    // Create page with the selected profile
    page = new MyWebPage(profile, webView);

    configurePage(page);


    webView->setPage(page);
    setupCustomContextMenu(webView, profile);

    // Add new tab
    int tabIndex = m_tabWidget->addTab(webView, "New Tab");
    showWebViews();
    m_tabWidget->setCurrentIndex(tabIndex);
    // Load the URL
    //webView->setUrl(QUrl(url));
    QString finalUrl;
    QString processedUrl = url;
    // Add https:// if no protocol is specified
    if (!processedUrl.startsWith("http://") && !processedUrl.startsWith("https://") && !processedUrl.startsWith("file://")) {
        processedUrl = "https://" + processedUrl;
    }
    QUrl testUrl(processedUrl);
    // Check if it looks like a URL (has dots, no spaces, etc.)
    if (testUrl.isValid() && processedUrl.contains('.') && !processedUrl.contains(' ')) {
        finalUrl = processedUrl;
    } else {
        // Treat as search term - use URLBar's selected search engine
        QUrl searchBase = m_urlBar->getSearchUrl();
        QString searchTerm = QUrl::toPercentEncoding(url);
        finalUrl = searchBase.toString() + searchTerm;
    }
    // Load the final URL
    webView->setUrl(QUrl(finalUrl));
    // Connect to the destroyed signal to clean up the profile
    connect(webView, &QObject::destroyed, this, [this, webView]() {
        cleanupTabProfile(webView);
    });

}

void MainWindow::createNewTab() {
    QWebEngineView *webView = new QWebEngineView(m_tabWidget);
    // URL sync connection
    connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
        if (QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            if (sender() == currentView) {
                m_urlBar->setUrl(url.toString());
            }
        }
    });
    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        int tabIndex = m_tabWidget->indexOf(webView);
        if (tabIndex != -1) {
            QString displayTitle = title.isEmpty() ? "New Tab" : title;
            if (displayTitle.length() > 20) {
                displayTitle = displayTitle.left(17) + "...";
            }
            m_tabWidget->setTabText(tabIndex, displayTitle);

        }
    });
    // Determine which profile to use
    QWebEngineProfile *profile;



    if (m_usingNamedProfiles && !m_selectedProfileName.isEmpty()) {
        profile = getOrCreateNamedProfile(m_selectedProfileName);
        m_tabNamedProfiles[webView] = m_selectedProfileName;
    } else if (m_usingSeparateProfiles) {
        profile = createProfileForTab();
        m_tabProfiles[webView] = profile;

    } else {
        profile = m_webProfile;
    }


    // Create page with the selected profile
    page = new MyWebPage(profile, webView);
    configurePage(page);
    webView->setPage(page);
    setupCustomContextMenu(webView, profile);

    // Add new tab
    int tabIndex = m_tabWidget->addTab(webView, "New Tab");
    showWebViews();
    m_tabWidget->setCurrentIndex(tabIndex);
    // Load the selected search engine URL
    QUrl searchEngineUrl = m_urlBar->getSearchUrl();
    webView->setUrl(searchEngineUrl);

    connect(webView, &QWebEngineView::iconChanged, this, [this, webView](const QIcon &icon) {


        // Update the tab icon
        int tabIndex = m_tabWidget->indexOf(webView);
        if (tabIndex != -1) {
            m_tabWidget->setTabIcon(tabIndex, icon);
        }
    });

    // Connect to the destroyed signal to clean up the profile
    connect(webView, &QObject::destroyed, this, [this, webView]() {
        cleanupTabProfile(webView);
    });
}


void MainWindow::cleanupTempProfiles() {
    //Do not perform cleaning if checkbox unchecked
    QSettings settings;
    bool shouldCleanup = settings.value("cleanupTempProfilesOnStartup", false).toBool();
    if (!shouldCleanup) {
        return;
    }

    QString profilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
    QDir profilesDir(profilesPath);
    if (!profilesDir.exists()) return;

    // Get ALL entries in profiles directory
    QStringList allEntries = profilesDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const QString& entry : allEntries) {
        // Skip the "named" directory which contains named profiles
        if (entry == "named") continue;

        // Skip the "sessions" directory which contains session symlinks
        if (entry == "sessions") continue;

        QString fullPath = profilesDir.absoluteFilePath(entry);
        QFileInfo info(fullPath);

        // If it's a directory (not a symlink) and no symlinks point to it -> DELETE
        if (info.isDir() && !info.isSymLink() && !hasSymlinksPointingTo(fullPath)) {
            QDir dir(fullPath);
            if (dir.removeRecursively()) {
                // Successfully removed
            }
        }
    }
}




bool MainWindow::hasSymlinksPointingTo(const QString& targetPath) {
    QString profilesPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/profiles";
    QDir profilesDir(profilesPath);

    // Get all Session_* entries (these could be symlinks)
    QStringList sessions = profilesDir.entryList(QStringList() << "Session_*", QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const QString& session : sessions) {
        QFileInfo info(profilesDir.absoluteFilePath(session));
        if (info.isSymLink() && info.symLinkTarget() == targetPath) {
            return true;
        }
    }
    return false;
}

void MainWindow::configureBrowserSettings(QWebEngineProfile* profile)
{
    // === Profile Settings ===


    // Set user agent (uncomment to override default)
    // profile->setHttpUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

    // Enable persistent storage (cookies, cache, etc.)
    // profile->setPersistentStoragePath(QDir::homePath() + "/.myapp/browser_data");
    // profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    // Configure cache
    // profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    // profile->setCachePath(QDir::homePath() + "/.myapp/browser_cache");

    // Set download path


    QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Jasmine";



#ifdef FLATPAK_BUILD
    profile->setDownloadPath(JASMINE_CONSTANTS::downloadsDirPath);
#else
    profile->setDownloadPath(JASMINE_CONSTANTS::downloadsDirPath);
#endif

    // === General WebEngine Settings ===
    QWebEngineSettings* settings = profile->settings();

    // --- JavaScript ---
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true); // Enable JavaScript
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true); // Block popups (set true for compatibility, but can be abused)
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false); // v Allow JS clipboard access (potential privacy risk)
    settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false); // v Allow JS paste (potential privacy risk)
    settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false); // Allow JS to activate windows (can be abused for popups)
    settings->setAttribute(QWebEngineSettings::ReadingFromCanvasEnabled, true); //  Allow reading from canvas (can be used for fingerprinting)

    // --- Content ---
    settings->setAttribute(QWebEngineSettings::AutoLoadImages, true); //  Load images automatically
    // Enable auto loading of icons for pages (favicons)
    settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true); // Enable WebGL (potential fingerprinting vector)
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true); // Enable plugins (can be a security risk)
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true); // Allow fullscreen API
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, true); // Enable built-in PDF viewer

    // --- Privacy & Security ---
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true); // Enable HTML5 local storage (can be used for tracking)
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false); // Allow local files to access remote URLs (security risk)
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false); //  Allow local files to access other local files (security risk)
    settings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true); // Enable XSS protection
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false); // Disable hyperlink auditing (privacy improvement)
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true); //  Enable DNS prefetching (improves speed, but can leak visited domains)

    // --- Media ---
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false); // Allow autoplay (can be annoying)
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true); // Allow screen capture (security risk if abused)
    settings->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly, true); // Restrict WebRTC to public interfaces (privacy improvement)

    // --- UI/UX ---
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, true); // Show scrollbars
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false); // Do not focus on navigation by default
    settings->setAttribute(QWebEngineSettings::PrintElementBackgrounds, true); // Print backgrounds
    settings->setAttribute(QWebEngineSettings::TouchIconsEnabled, true); // Enable touch icons
    settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true); // Enable smooth scrolling
    settings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, false); // Disable spatial navigation (enable for TV-like interfaces)
    settings->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain, false); // Exclude links from focus chain
    settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, true); // Show error pages
    settings->setAttribute(QWebEngineSettings::ForceDarkMode, false); // Do not force dark mode

    // --- Fonts ---
    settings->setFontFamily(QWebEngineSettings::StandardFont, "Arial");
    settings->setFontFamily(QWebEngineSettings::FixedFont, "Courier New");
    settings->setFontFamily(QWebEngineSettings::SerifFont, "Times New Roman");
    settings->setFontFamily(QWebEngineSettings::SansSerifFont, "Arial");
    settings->setFontSize(QWebEngineSettings::DefaultFontSize, 16);
    settings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, 13);
    settings->setFontSize(QWebEngineSettings::MinimumFontSize, 10);

    // --- Additional/Optional Settings (commented out for reference) ---

    // Enable accelerated 2D canvas for better performance
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);



    // Enable hyperlink auditing (ping attribute, optional)
    //settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, true);

    // Enable navigation on drop (drag-and-drop URLs)
    //settings->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, true);

    // Set Unknown URL Scheme Policy (handle custom protocols, e.g., mailto:)
    //settings->setUnknownUrlSchemePolicy(QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction);

    // Optionally, set image animation policy (Qt 6.8+)
    // settings->setImageAnimationPolicy(QWebEngineSettings::ImageAnimationPolicy::Allow);


    // --- Custom URL Scheme Handlers (uncomment and implement as needed) ---
    // m_webProfile->installUrlSchemeHandler("app", new AppSchemeHandler(this));

    // --- Client Certificate Storage (uncomment and implement as needed) ---
    // m_webProfile->setClientCertificateStore(...);

    // --- Custom Request Interceptor (uncomment and implement as needed) ---
    // RequestInterceptor* interceptor = new RequestInterceptor(this);
    // m_webProfile->setUrlRequestInterceptor(interceptor);

    // --- Proxy Configuration (uncomment and configure as needed) ---
    /*
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("proxy.example.com");
    proxy.setPort(8080);
    QNetworkProxy::setApplicationProxy(proxy);
    */

    // --- Spell Checker (Qt 5.8+) ---
    /*
    #if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    profile->setSpellCheckEnabled(true);
    profile->setSpellCheckLanguages(QStringList() << "en-US");
    #endif
    */

    // --- Security Notes ---
    // - Enabling LocalContentCanAccessRemoteUrls and LocalContentCanAccessFileUrls can expose local files or allow local HTML to make network requests. Only enable if you trust your content.
    // - PluginsEnabled and WebGLEnabled can increase attack surface or fingerprinting risk.
    // - Allowing JavaScript clipboard access, screen capture, or autoplay can be abused by malicious sites.
    // - Review all enabled features for your application's threat model.
}



void MainWindow::injectAdBlockScript(MyWebPage *page)
{
    if (!page || !m_adBlocker || !m_adBlocker->isEnabled()) return;

    // Connect to the loadFinished signal
    connect(page, &QWebEnginePage::loadFinished, this, [this, page](bool ok) {
        // Check again in case ad blocker was disabled between page creation and load completion
        if (ok && m_adBlocker && m_adBlocker->isEnabled()) {
            // Inject the ad-blocking script
            page->runJavaScript(AdBlockScript::getScript());
        }
    });
}

/////////////////////////////////////////////
QWebEngineProfile* MainWindow::getOrCreateNamedProfile(const QString& name) {
    // Return existing profile if we have it
    if (m_namedProfiles.contains(name)) {
        return m_namedProfiles[name];
    }

    // Create a new named profile
    QString profilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                          "/profiles/named/" + name;

    QWebEngineProfile* profile = new QWebEngineProfile(name, this);
    // add attributes
    configureProfile(profile);

    // Configure the profile
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->setPersistentStoragePath(profilePath);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setCachePath(profilePath + "/cache");

    // Store in our map
    m_namedProfiles[name] = profile;

    return profile;
}

void MainWindow::saveNamedProfilesData() {
    QSettings settings;

    // Save list of profile names
    QStringList profileNames = m_namedProfiles.keys();
    settings.setValue("namedProfiles/profiles", profileNames);

    // Save whether named profiles are being used
    settings.setValue("namedProfiles/usingNamedProfiles", m_usingNamedProfiles);

    // Save selected profile
    //settings.setValue("namedProfiles/selectedProfile", m_selectedProfileName);
}

void MainWindow::loadNamedProfilesData() {
    QSettings settings;

    // Load whether named profiles are being used
    m_usingNamedProfiles = settings.value("namedProfiles/usingNamedProfiles", false).toBool();

    // Load selected profile
    //m_selectedProfileName = settings.value("namedProfiles/selectedProfile", "").toString();
    m_selectedProfileName = "";

    // Load list of profile names
    profileNames = settings.value("namedProfiles/profiles", QStringList()).toStringList();

    // Create profile objects for each name
    for (const QString& name : profileNames) {
        getOrCreateNamedProfile(name);
    }
}

void MainWindow::setupNamedProfilesUI() {

    profileSelector->clear();
    profileSelector->addItem("Default", "");



    for (const QString& name : m_namedProfiles.keys()) {
        profileSelector->addItem(name, name);
    }

    // Add special items
    profileSelector->addItem("New Profile...", "new");
    profileSelector->addItem("Manage Profiles...", "manage");

    // Set current profile
    if (!m_selectedProfileName.isEmpty()) {
        int index = profileSelector->findData(m_selectedProfileName);
        if (index >= 0) {
            profileSelector->setCurrentIndex(index);
        }
    }
    profileSelector->setEnabled(true);

    // Connect signals
    connect(profileSelector, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        QString data = profileSelector->itemData(index).toString();

        if (data == "new") {
            // Create new profile
            bool ok;
            QString name = QInputDialog::getText(this, "New Profile",
                                                 "Enter profile name:",
                                                 QLineEdit::Normal, "", &ok);
            if (ok && !name.isEmpty()) {
                // Create the profile
                getOrCreateNamedProfile(name);

                // Update the selector
                profileSelector->insertItem(profileSelector->count() - 2, name, name);
                profileSelector->setCurrentIndex(profileSelector->count() - 3);

                // Set as selected
                m_selectedProfileName = name;
                m_usingNamedProfiles = true;

                // Save settings
                saveNamedProfilesData();
            }

            // Reset to previous selection
            int prevIndex = profileSelector->findData(m_selectedProfileName);
            if (prevIndex >= 0) {
                profileSelector->setCurrentIndex(prevIndex);
            } else {
                profileSelector->setCurrentIndex(0);
            }
        }
        else if (data == "manage") {
            showProfileManager();

            // Reset to previous selection
            int prevIndex = profileSelector->findData(m_selectedProfileName);
            if (prevIndex >= 0) {
                profileSelector->setCurrentIndex(prevIndex);
            } else {
                profileSelector->setCurrentIndex(0);
            }
        }
        else {
            // Normal profile selection
            m_selectedProfileName = data;
            m_usingNamedProfiles = !data.isEmpty();
            saveNamedProfilesData();
        }
    });

}


void MainWindow::showProfileManager() {
    QDialog dialog(this);
    dialog.setWindowTitle("Manage Profiles");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Create list widget
    QListWidget* profileList = new QListWidget(&dialog);
    profileList->addItems(m_namedProfiles.keys());
    layout->addWidget(profileList);

    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* newButton = new QPushButton("New", &dialog);
    QPushButton* deleteButton = new QPushButton("Delete", &dialog);
    QPushButton* cleanButton = new QPushButton("Clean Profile", &dialog);
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(cleanButton);
    layout->addLayout(buttonLayout);

    // Add close button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);

    // Connect new button
    connect(newButton, &QPushButton::clicked, &dialog, [this, profileList]() {
        // Create new profile
        bool ok;
        QString name = QInputDialog::getText(this, "New Profile",
                                             "Enter profile name:",
                                             QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            // Check if profile already exists
            if (m_namedProfiles.contains(name)) {
                QMessageBox::warning(this, "Profile Exists",
                                     "A profile with this name already exists.");
                return;
            }

            // Create the profile
            getOrCreateNamedProfile(name);

            // Update the list
            profileList->addItem(name);

            // Update the selector in main window
            profileSelector->insertItem(profileSelector->count() - 2, name, name);

            // Save settings
            saveNamedProfilesData();
        }
    });

    // Connect delete button
    connect(deleteButton, &QPushButton::clicked, &dialog, [this, profileList]() {
        QListWidgetItem* item = profileList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "No Profile Selected",
                                 "Please select a profile to delete.");
            return;
        }

        QString name = item->text();

        // Get profile directory path
        QString namedProfilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                                       .filePath("profiles/named/" + name);
        QString sessionsDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                                  .filePath("profiles/sessions");

        // Check if profile is in use by checking for symlinks
        QDir dir(sessionsDir);
        if (dir.exists()) {
            // Get all session directories
            QStringList sessions = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

            // Check each session for symlinks to this profile
            for (const QString& session : sessions) {
                QString sessionPath = QDir(sessionsDir).filePath(session);
                QFileInfo linkInfo(sessionPath);

                // If it's a symlink and points to our profile, the profile is in use
                if (linkInfo.isSymLink() && linkInfo.symLinkTarget() == namedProfilePath) {
                    QMessageBox::warning(this, "Profile In Use",
                                         "Profile '" + name + "' is currently in use by active sessions and cannot be deleted.");
                    return;
                }
            }
        }

        // Confirm deletion
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Profile",
                                                                  "Are you sure you want to delete profile '" + name + "'?\n"
                                                                                                                       "This will permanently delete all data for this profile.",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Remove from map (m_namedProfiles is a QMap<QString, QWebEngineProfile*>)
            QWebEngineProfile* profile = m_namedProfiles.take(name);
            if (profile) {
                // Delete the profile object
                profile->deleteLater();

                // Delete the profile directory
                QDir profileDir(namedProfilePath);
                if (profileDir.exists()) {
                    if (!profileDir.removeRecursively()) {
                        QMessageBox::warning(this, "Delete Failed",
                                             "Failed to delete profile directory. Some files may be in use.");
                    }
                }

                // Remove from list
                delete item;

                // Remove from selector in main window
                for (int i = 0; i < profileSelector->count(); i++) {
                    if (profileSelector->itemData(i).toString() == name) {
                        profileSelector->removeItem(i);
                        break;
                    }
                }

                // If this was the selected profile, switch to default
                if (m_selectedProfileName == name) {
                    m_selectedProfileName = "";
                    m_usingNamedProfiles = false;
                    profileSelector->setCurrentIndex(0);
                }

                // Save settings
                saveNamedProfilesData();
            }
        }
    });

    // Connect clean button
    connect(cleanButton, &QPushButton::clicked, &dialog, [this, profileList]() {
        QListWidgetItem* item = profileList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "No Profile Selected",
                                 "Please select a profile to clean.");
            return;
        }

        QString name = item->text();
        QWebEngineProfile* profile = m_namedProfiles.value(name);
        if (!profile) return;

        // Confirm cleaning
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Clean Profile",
                                                                  "Are you sure you want to clean all data for profile '" + name + "'?\n"
                                                                                                                                   "This will delete cookies, cache, and browsing history.",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Clean all data directly
            profile->cookieStore()->deleteAllCookies();
            profile->clearHttpCache();
            profile->clearAllVisitedLinks();

            QMessageBox::information(this, "Profile Cleaned",
                                     "All data for profile '" + name + "' has been cleaned.");
        }
    });

    dialog.resize(400, 300);
    dialog.exec();
}


void MainWindow::handleFullScreenRequest(QWebEngineFullScreenRequest request)
{

    QWebEngineView* webView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget());
    if (!webView)
        return;

    request.accept();

    if (request.toggleOn()) {
        enterTheaterMode(webView);
    } else {
        exitTheaterMode(webView);
    }
}

void MainWindow::enterTheaterMode(QWebEngineView* webView)
{
    if (m_isInTheaterMode || !webView)
        return;

    m_isInTheaterMode = true;
    m_theaterWebView = webView;
    m_lastTabIndex = m_tabWidget->indexOf(webView);

    // Maximize the main window, but do not go fullscreen
    showMaximized();
    showWebViews();

    toolbar->setVisible(false);
    menuBar()->setVisible(false);
    m_urlBar->setVisible(false);
    statusBar()->setVisible(false);

    m_tabWidget->tabBar()->setVisible(false);

}

void MainWindow::exitTheaterMode(QWebEngineView* webView)
{
    if (!m_isInTheaterMode || !m_theaterWebView)
        return;

    //showNormal();
    showDashboard();
    showWebViews();

    toolbar->setVisible(true);
    menuBar()->setVisible(true);
    m_urlBar->setVisible(false);
    statusBar()->setVisible(true);
    m_tabWidget->tabBar()->setVisible(true);


    // Clean up state
    m_isInTheaterMode = false;
    m_theaterWebView = nullptr;
}




void MainWindow::configureProfile(QWebEngineProfile *profile)
{
    profile->setUrlRequestInterceptor(interceptor);
    if (m_adBlocker && m_adBlocker->isEnabled()) {
        profile->setUrlRequestInterceptor(m_adBlocker);
        //setupVideoAdBlocking(profile);
        //auto cosmetic = new CosmeticFilterInjector(profile);

    }

    connect(profile, &QWebEngineProfile::downloadRequested,
            downloadManager, &DownloadManager::handleDownloadRequest,Qt::UniqueConnection);

    configureBrowserSettings(profile);

}

void MainWindow::configurePage(MyWebPage *page)
{
    connect(page, &QWebEnginePage::fullScreenRequested,
            this, &MainWindow::handleFullScreenRequest);
    /*

    connect(page, &QWebEnginePage::featurePermissionRequested,
            [page](const QUrl &requestingOrigin, QWebEnginePage::Feature feature) {
                static QSet<QPair<QUrl, QWebEnginePage::Feature>> pendingRequests;
                auto request = qMakePair(requestingOrigin, feature);

                if (pendingRequests.contains(request)) {
                    return;
                }

                pendingRequests.insert(request);

                switch(feature) {
                //case QWebEnginePage::Geolocation:

                //case QWebEnginePage::MediaAudioCapture:

                //case QWebEnginePage::MediaVideoCapture:

                //case QWebEnginePage::MediaAudioVideoCapture:

                //case QWebEnginePage::MouseLock:

                //case QWebEnginePage::DesktopVideoCapture:

                //case QWebEnginePage::DesktopAudioVideoCapture:

                //case QWebEnginePage::Notifications:
                    page->setFeaturePermission(requestingOrigin, feature, QWebEnginePage::PermissionGrantedByUser);
                    break;
                default:
                    break;
                }

                pendingRequests.remove(request);
            });
            */

    connect(page, &MyWebPage::newTabRequested, this, [this](QWebEngineView *view, QWebEngineProfile *){
            createNewTabWithUrlFromLink(view->url().toString(), view);
        });



    connect(page, &MyWebPage::newPopupRequested, this, [this](QWebEngineView *view, QWebEngineProfile *){
            createNewTabWithUrlFromLink(view->url().toString(), view);

        });

}


void MainWindow::onWebViewContextMenu(const QPoint &pos, QWebEngineProfile * /*profile*/)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    if (!view) return;

    QWebEngineContextMenuRequest *request = view->lastContextMenuRequest();
    if (!request) return;

    QMenu menu;

    // Navigation
    menu.addAction("Back", view, &QWebEngineView::back);
    menu.addAction("Forward", view, &QWebEngineView::forward);
    menu.addAction("Reload", view, &QWebEngineView::reload);
    menu.addAction("Stop", view, &QWebEngineView::stop);
    menu.addSeparator();

    // Edit actions
    QAction *cutAction = menu.addAction("Cut");
    QAction *copyAction = menu.addAction("Copy");
    QAction *pasteAction = menu.addAction("Paste");
    QAction *selectAllAction = menu.addAction("Select All");
    menu.addSeparator();

    // Link-specific actions (only show if there's a link)
    bool hasLink = request->linkUrl().isValid();
    QAction *copyLinkAction = nullptr;
    QAction *openTabAction = nullptr;
    QAction *searchAction = nullptr;

    //QAction *openWindowAction = nullptr;

    if (hasLink) {
        copyLinkAction = menu.addAction("Copy link address");
        openTabAction = menu.addAction("Open link in new tab(Preserves calling tab's profile)");
        //openWindowAction = menu.addAction("Open link in new window");
        menu.addSeparator();
    }

    bool hasSelectedText = !request->selectedText().trimmed().isEmpty();

    if (hasSelectedText) {
        searchAction = menu.addAction("Search " + m_urlBar->getCurrentEngine());
        menu.addSeparator();

    }

    // Check if context menu was triggered on media content (image, video, audio)
    bool hasMediaContent = (request->mediaType() != QWebEngineContextMenuRequest::MediaTypeNone);
    QAction *copyImageAction = nullptr;
    QAction *saveImageAction = nullptr;

    if (hasMediaContent && request->mediaType() == QWebEngineContextMenuRequest::MediaTypeImage) {
        copyImageAction = menu.addAction("Copy image");
        saveImageAction = menu.addAction("Save image");
        menu.addSeparator();
    }

    // Page actions
    menu.addAction("View page source", [view, this]() {
        devToolsView->resize(1000, 800);
        devToolsView->show();

        // Create a page for it using the same profile
        QWebEnginePage *sourcePage = new QWebEnginePage(view->page()->profile(), devToolsView);
        devToolsView->setPage(sourcePage);

        // Attach it as the "devtools" page (Qt uses this connection internally)
        view->page()->setDevToolsPage(sourcePage);

        // Trigger the built-in View Source action
        view->page()->triggerAction(QWebEnginePage::ViewSource);
    });

    menu.addAction("Inspect element", [view, this]() {
            QWebEnginePage *devToolsPage = new QWebEnginePage(view->page()->profile(), devToolsView);
            devToolsView->setPage(devToolsPage);
            devToolsView->resize(1000, 800);
            devToolsView->show();

            // attach devtools to this new page
            view->page()->setDevToolsPage(devToolsPage);

            // trigger inspect mode
            view->page()->triggerAction(QWebEnginePage::InspectElement);
        view->page()->triggerAction(QWebEnginePage::InspectElement);
    });

    QAction *selected = menu.exec(view->mapToGlobal(pos));
    if (!selected) {
        // User canceled the menu, do nothing
        return;
    }

    // Handle selections
    if (selected == cutAction) {
        view->triggerPageAction(QWebEnginePage::Cut);
    }
    else if (selected == copyAction) {
        view->triggerPageAction(QWebEnginePage::Copy);
    }
    else if (selected == pasteAction) {
        view->triggerPageAction(QWebEnginePage::Paste);
    }
    else if (selected == selectAllAction) {
        view->triggerPageAction(QWebEnginePage::SelectAll);
    }
    else if (selected == copyLinkAction && hasLink) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(request->linkUrl().toString());
    }
    else if (selected == openTabAction && hasLink) {
        createNewTabWithUrlFromLink(request->linkUrl().toString(), view);
    }
    //else if (selected == openWindowAction && hasLink) {
        // Implement open in new window functionality
    //    createNewTabWithUrlFromLink(request->linkUrl().toString(), view);
    //}
    else if (selected == copyImageAction && hasMediaContent) {
        view->triggerPageAction(QWebEnginePage::CopyImageToClipboard);
    }

    else if (selected == saveImageAction && hasMediaContent) {
        QUrl imageUrl = request->mediaUrl();
        if (imageUrl.isValid()) {
            // This will trigger the downloadRequested signal automatically
            view->page()->download(imageUrl);
        }
    }

    if (selected == searchAction) {

        createNewTabWithUrlFromLink(request->selectedText(),view);

    }
}

void MainWindow::setupCustomContextMenu(QWebEngineView *view, QWebEngineProfile *profile)
{
    if (!view) return;

    // Allow custom context menu
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the signal to the slot
    connect(view, &QWidget::customContextMenuRequested, this,
                [this, view, profile](const QPoint &pos){
                    onWebViewContextMenu(pos, profile);
                });
}


void MainWindow::createNewTabWithUrlFromLink(const QString &url, QWebEngineView *originView) {
    // Determine profile
    QWebEngineProfile *profile = nullptr;

    if (originView) {
        profile = originView->page()->profile();  // inherit profile directly
    } else {
        profile = m_webProfile; // fallback shared
    }

    // Configure profile safely
    configureProfile(profile);

    // Create new web view and page
    QWebEngineView *webView = new QWebEngineView(m_tabWidget);

    connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
           if (QWebEngineView *currentView = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
               if (sender() == currentView) {
                   m_urlBar->setUrl(url.toString());
               }
           }
       });

    connect(webView, &QWebEngineView::titleChanged, this, [this, webView](const QString &title) {
        int tabIndex = m_tabWidget->indexOf(webView);
        if (tabIndex != -1) {
            QString displayTitle = title.isEmpty() ? "New Tab" : title;
            if (displayTitle.length() > 20) {
                displayTitle = displayTitle.left(17) + "...";
            }
            m_tabWidget->setTabText(tabIndex, displayTitle);

        }
    });

    MyWebPage *page = new MyWebPage(profile, webView);

    configurePage(page);
    webView->setPage(page);
    setupCustomContextMenu(webView, profile);

    // Keep profile maps in sync for proper cleanup
    if (originView) {
        if (m_tabNamedProfiles.contains(originView)) {
            QString name = m_tabNamedProfiles[originView];
            m_tabNamedProfiles[webView] = name; // inherit named profile
        } else if (m_tabProfiles.contains(originView)) {
            m_tabProfiles[webView] = m_tabProfiles[originView]; // inherit private profile
        } else {
            // shared profile → mark as nullptr for cleanup logic
            //m_tabProfiles[webView] = nullptr;
        }
    } else {
        // No origin → shared profile
        //m_tabProfiles[webView] = nullptr;
    }

    // Add tab
    int tabIndex = m_tabWidget->addTab(webView, "New Tab");
    showWebViews();

    m_tabWidget->setCurrentIndex(tabIndex);

    // Load the URL
    //first diff between legit url and search term
    QString finalUrl;
    QString processedUrl = url;
    // Add https:// if no protocol is specified
    if (!processedUrl.startsWith("http://") && !processedUrl.startsWith("https://") && !processedUrl.startsWith("file://")) {
        processedUrl = "https://" + processedUrl;
    }
    QUrl testUrl(processedUrl);
    // Check if it looks like a URL (has dots, no spaces, etc.)
    if (testUrl.isValid() && processedUrl.contains('.') && !processedUrl.contains(' ')) {
        finalUrl = processedUrl;
    } else {
        // Treat as search term - use URLBar's selected search engine
        QUrl searchBase = m_urlBar->getSearchUrl();
        QString searchTerm = QUrl::toPercentEncoding(url);
        finalUrl = searchBase.toString() + searchTerm;
    }

    webView->setUrl(QUrl(finalUrl));
    connect(webView, &QWebEngineView::iconChanged, this, [this, webView](const QIcon &icon) {


            // Update the tab icon
            int tabIndex = m_tabWidget->indexOf(webView);
            if (tabIndex != -1) {
                m_tabWidget->setTabIcon(tabIndex, icon);
            }
    });


    // Ensure proper cleanup on tab destruction
    connect(webView, &QObject::destroyed, this, [this, webView]() {
        cleanupTabProfile(webView);
    });

}




void MainWindow::setupVideoAdBlocking(QWebEngineProfile *profile)
{
    // Check if scripts already exist to avoid duplicates
    static bool scriptsInitialized = false;
    if (scriptsInitialized) {
        return;
    }
    scriptsInitialized = true;

    // YouTube-specific ad blocking
    QWebEngineScript youtubeScript;
    youtubeScript.setSourceCode(R"(
        (function() {
            function blockYouTubeAds() {
                // Skip buttons
                const skipBtn = document.querySelector('.ytp-ad-skip-button, .ytp-ad-skip-button-modern');
                if (skipBtn) {
                    skipBtn.click();
                }

                // Remove ad containers
                const adContainers = document.querySelectorAll('.video-ads, .ad-container, .ytp-ad-module, .ytp-ad-overlay-container');
                adContainers.forEach(container => {
                    container.style.display = 'none';
                    container.remove();
                });

                // Block ad markers in progress bar
                const adMarkers = document.querySelectorAll('.ytp-ad-marker-container');
                adMarkers.forEach(marker => marker.remove());
            }

            // Run frequently
            setInterval(blockYouTubeAds, 500);
            document.addEventListener('DOMContentLoaded', blockYouTubeAds);

            // MutationObserver for dynamic content
            const observer = new MutationObserver(blockYouTubeAds);
            observer.observe(document.body, { childList: true, subtree: true });
        })();
    )");

    youtubeScript.setName("YouTube_Ad_Blocker");
    youtubeScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    youtubeScript.setWorldId(QWebEngineScript::MainWorld);
    youtubeScript.setRunsOnSubFrames(true);
    profile->scripts()->insert(youtubeScript);

    // General video ad blocking
    QWebEngineScript generalScript;
    generalScript.setSourceCode(R"(
        document.addEventListener('DOMContentLoaded', function() {
            const videos = document.querySelectorAll('video[autoplay]');
            videos.forEach(video => {
                if (video.duration <= 60 ||
                    video.src.includes('ad') ||
                    video.parentElement.innerHTML.toLowerCase().includes('ad')) {
                    video.pause();
                    video.remove();
                }
            });
        });
    )");

    generalScript.setName("General_Video_Ad_Blocker");
    generalScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    generalScript.setWorldId(QWebEngineScript::MainWorld);
    generalScript.setRunsOnSubFrames(true);
    profile->scripts()->insert(generalScript);

}


void MainWindow::highlightButtons(bool highlighted)
{
    if(highlighted) {
        m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/repeat.svg"));


        toggleButton->setStyleSheet(
                    "QToolButton {"
                    "   background-color: #b0b0b0;"  /* Darker gray-beige */
                    "   color: white;"
                    "   border-radius: 4px;"
                    "   padding: 4px 8px;"
                    "   border: none;"
                    "}"
                    "QToolButton:hover {"
                    "   background-color: #a0a0a0;"
                    "   color: white;"
                    "}"
                    );

        m_addCurrentSessionAction->setIcon(QIcon(":/resources/icons-white/save.svg"));

        // Find the button and style it
        QToolButton* saveButton = qobject_cast<QToolButton*>(toolbar->widgetForAction(m_addCurrentSessionAction));
        if (saveButton) {
            saveButton->setStyleSheet(
                        "QToolButton {"
                        "   background-color: #b0b0b0;"  /* Darker gray-beige */
                        "   color: white;"
                        "   border-radius: 4px;"
                        "   padding: 4px 8px;"
                        "   border: none;"
                        "}"
                        "QToolButton:hover {"
                        "   background-color: #a0a0a0;"  /* Same darker blue on hover */
                        "   color: white;"
                        "}"
                        );
        }

        m_addCurrentWebsiteAction->setIcon(QIcon(":/resources/icons-white/plus.svg"));

        // Find the button and style it
        QToolButton* websiteButton = qobject_cast<QToolButton*>(toolbar->widgetForAction(m_addCurrentWebsiteAction));
        if (websiteButton) {
            //websiteButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // Show text beside icon
            websiteButton->setStyleSheet(
                        "QToolButton {"
                        "   background-color: #b0b0b0;"  /* Darker gray-beige */
                        "   color: white;"
                        "   border-radius: 4px;"
                        "   padding: 4px 8px;"
                        "   border: none;"
                        "}"
                        "QToolButton:hover {"
                        "   background-color: #a0a0a0;"
                        "   color: white;"
                        "}"
                        );
        }
    }else{
        if(!m_isDarkTheme) m_toggleViewAction->setIcon(QIcon(":/resources/icons/repeat.svg"));

        toggleButton->setStyleSheet("");


        if(!m_isDarkTheme) m_addCurrentSessionAction->setIcon(QIcon(":/resources/icons/save.svg"));

        // Find the button and style it
        QToolButton* saveButton = qobject_cast<QToolButton*>(toolbar->widgetForAction(m_addCurrentSessionAction));
        if (saveButton) {
            saveButton->setStyleSheet("");
        }

        if(!m_isDarkTheme) m_addCurrentWebsiteAction->setIcon(QIcon(":/resources/icons/plus.svg"));

        // Find the button and style it
        QToolButton* websiteButton = qobject_cast<QToolButton*>(toolbar->widgetForAction(m_addCurrentWebsiteAction));
        if (websiteButton) {
            //websiteButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);  // Show text beside icon
            websiteButton->setStyleSheet("");
                        }

    }

}


///////radio
void MainWindow::createRadioTab()
{
    m_radioScrollArea = new QScrollArea();
    m_radioScrollArea->setWidgetResizable(true);
    m_radioScrollArea->setFrameShape(QFrame::NoFrame);
    m_radioContainer = new QWidget();
    m_radioGrid = new QGridLayout(m_radioContainer);
    m_radioGrid->setSpacing(12);
    m_radioGrid->setContentsMargins(16, 16, 16, 16);
    m_radioGrid->setSizeConstraint(QLayout::SetFixedSize);
    for (int i = 0; i < 3; ++i) {
        m_radioGrid->setColumnStretch(i, 0);
    }
    m_radioScrollArea->setWidget(m_radioContainer);

    m_leftPanelTabs->addTab(m_radioScrollArea, createRotatedIcon(":/resources/icons/radio.svg"), QString());
    m_leftPanelTabs->tabBar()->setTabToolTip(2, "Radio");

    QFrame* radioDetailsPanel = createRadioDetailPanel();
    radioDetailsPanel->setMinimumWidth(200);
    detailsStack->addWidget(radioDetailsPanel);  // Index 2

    // Load saved stations
    //loadRadioStations();

    // Update grid if there are stations
    if (!m_radioStations.isEmpty()) {
        //updateRadioGrid();
    }
}

QFrame* MainWindow::createRadioDetailPanel() {
    QFrame* panel = new QFrame();
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setObjectName("radioDetailsPanel");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);

    // Title
    QLabel* titleLabel = new QLabel("Radio Station");
    titleLabel->setObjectName("detailsPanelTitle");
    layout->addWidget(titleLabel);

    // Form layout for station details
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Station icon with generate button
    QHBoxLayout* iconLayout = new QHBoxLayout();

    // Station icon (display only, like website favicon)
    m_radioIconLabel = new QLabel();
    m_radioIconLabel->setFixedSize(64, 64);
    m_radioIconLabel->setAlignment(Qt::AlignCenter);
    m_radioIconLabel->setFrameShape(QFrame::StyledPanel);
    m_radioIconLabel->setObjectName("radioIconPreview");
    m_radioIconLabel->setScaledContents(true);


    m_radioGenerateIconButton = new QPushButton();
    m_radioGenerateIconButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_radioGenerateIconButton->setFixedSize(24, 24);
    m_radioGenerateIconButton->setToolTip("Generate icon from station name");

    iconLayout->addWidget(m_radioIconLabel);
    iconLayout->addWidget(m_radioGenerateIconButton);
    iconLayout->addStretch();

    formLayout->addRow("Icon:", iconLayout);

    formLayout->addRow("Icon:", iconLayout);

    // Station name - QLineEdit (editable)
    m_radioNameEdit = new QLineEdit();
    m_radioNameEdit->setPlaceholderText("Station name");
    formLayout->addRow("Name:", m_radioNameEdit);

    // Stream URL - QLineEdit (editable)
    m_radioStreamUrlEdit = new QLineEdit();
    m_radioStreamUrlEdit->setPlaceholderText("Stream URL");
    formLayout->addRow("Stream URL:", m_radioStreamUrlEdit);

    // Country - QLineEdit (editable)
    m_radioCountryEdit = new QLineEdit();
    m_radioCountryEdit->setPlaceholderText("Country code (e.g., GR)");
    formLayout->addRow("Country:", m_radioCountryEdit);

    // Genre - QLineEdit (editable)
    m_radioGenreEdit = new QLineEdit();
    m_radioGenreEdit->setPlaceholderText("Genre (e.g., Rock, News, Pop)");
    formLayout->addRow("Genre:", m_radioGenreEdit);

    // Bitrate - QLineEdit (editable)
    m_radioBitrateEdit = new QLineEdit();
    m_radioBitrateEdit->setPlaceholderText("Bitrate in kbps");
    formLayout->addRow("Bitrate:", m_radioBitrateEdit);

    // Codec - QLineEdit (editable)
    m_radioCodecEdit = new QLineEdit();
    m_radioCodecEdit->setPlaceholderText("Codec (MP3, AAC, etc.)");
    formLayout->addRow("Codec:", m_radioCodecEdit);

    // Comments - QTextEdit (like website panel)
    m_radioCommentsEdit = new QTextEdit();
    m_radioCommentsEdit->setPlaceholderText("Optional comments about this station");
    m_radioCommentsEdit->setMinimumHeight(80);
    formLayout->addRow("Comments:", m_radioCommentsEdit);

    layout->addLayout(formLayout);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch();
    // Add Update and Delete buttons (like website panel)

    QPushButton* m_radioAddButton = new QPushButton("Add");
    m_radioAddButton->setToolTip("Add new  station");
    m_radioAddButton->setObjectName("primaryButton");

    m_radioUpdateButton = new QPushButton("Update");
    m_radioUpdateButton->setToolTip("Update station details");
    m_radioUpdateButton->setObjectName("secondaryButton");

    m_radioDeleteButton = new QPushButton("Delete");
    m_radioDeleteButton->setToolTip("Delete this station");
    m_radioDeleteButton->setObjectName("secondaryButton");

    m_radioClearButton = new QPushButton("Clear");
    m_radioClearButton->setToolTip("Clear form");
    m_radioClearButton->setObjectName("secondaryButton");

    buttonLayout->addWidget(m_radioAddButton);
    buttonLayout->addWidget(m_radioUpdateButton);
    buttonLayout->addWidget(m_radioDeleteButton);
    buttonLayout->addWidget(m_radioClearButton);
    buttonLayout->addStretch();

    layout->addSpacing(16);
    layout->addLayout(buttonLayout);
    layout->addStretch();


    QHBoxLayout* buttonLayoutX = new QHBoxLayout();
    buttonLayoutX->setSpacing(8);
    buttonLayoutX->addStretch();
    m_radioPlayButton = new QPushButton("Play");
    m_radioPlayButton->setToolTip("Play this radio station");
    m_radioPlayButton->setObjectName("primaryButton");

    m_radioStopButton = new QPushButton("Stop");
    m_radioStopButton->setToolTip("Stop playback");
    m_radioStopButton->setObjectName("secondaryButton");
    m_radioStopButton->setEnabled(false);

    buttonLayoutX->addWidget(m_radioPlayButton);
    buttonLayoutX->addWidget(m_radioStopButton);
    buttonLayoutX->addStretch();

    layout->addSpacing(16);
    layout->addLayout(buttonLayoutX);
    layout->addStretch();

    // Connect signals
    connect(m_radioPlayButton, &QPushButton::clicked, this, &MainWindow::onRadioPlayClicked);
    connect(m_radioStopButton, &QPushButton::clicked, this, &MainWindow::onRadioStopClicked);
    connect(m_radioUpdateButton, &QPushButton::clicked, this, &MainWindow::onRadioUpdateClicked);
    connect(m_radioDeleteButton, &QPushButton::clicked, this, &MainWindow::onRadioDeleteClicked);
    connect(m_radioClearButton, &QPushButton::clicked, this, &MainWindow::onRadioClearClicked);
    connect(m_radioAddButton, &QPushButton::clicked, this, &MainWindow::onRadioAddClicked);


    connect(m_radioGenerateIconButton, &QPushButton::clicked, [this]() {
        if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {
            QString stationName = m_radioStations[m_currentRadioIndex].name;
            // Generate a simple colored icon with first letter

            QIcon newIcon = generateRandomSvgIcon();
            QPixmap pixmap = newIcon.pixmap(64, 64);
            m_radioIconLabel->setPixmap(pixmap);

            // Save the generated icon
            QString iconPath = JASMINE_CONSTANTS::iconDir + "/" +
                              m_radioStations[m_currentRadioIndex].stationuuid + "_generated.png";
            pixmap.save(iconPath);

            // Update station with local path
            m_radioStations[m_currentRadioIndex].iconUrl = iconPath;
            saveRadioStations();
            updateRadioGrid();
        }
    });

    return panel;
}



// Slots for radio playback


void MainWindow::onRadioUpdateClicked()
{
    if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {
        // Update the station with values from form
        m_radioStations[m_currentRadioIndex].name = m_radioNameEdit->text().trimmed();
        m_radioStations[m_currentRadioIndex].streamUrl = m_radioStreamUrlEdit->text().trimmed();
        m_radioStations[m_currentRadioIndex].countrycode = m_radioCountryEdit->text().trimmed();
        m_radioStations[m_currentRadioIndex].genre = m_radioGenreEdit->text().trimmed();
        m_radioStations[m_currentRadioIndex].bitrate = m_radioBitrateEdit->text().trimmed().toInt();
        m_radioStations[m_currentRadioIndex].codec = m_radioCodecEdit->text().trimmed();
        m_radioStations[m_currentRadioIndex].comments = m_radioCommentsEdit->toPlainText().trimmed();

        saveRadioStations();
        updateRadioGrid();

        statusBar()->showMessage("Radio station updated.", 3000);
    }
}

void MainWindow::onRadioDeleteClicked()
{
    if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {
        m_radioStations.removeAt(m_currentRadioIndex);
        saveRadioStations();
        updateRadioGrid();

        if (m_radioStations.isEmpty()) {
            m_radioNameEdit->clear();
            m_radioStreamUrlEdit->clear();
            m_radioCountryEdit->clear();
            m_radioBitrateEdit->clear();
            m_radioCodecEdit->clear();
            m_radioCommentsEdit->clear();
            m_radioIconLabel->clear();
            m_currentRadioIndex = -1;
        } else {
            onRadioStationSelected(0);
        }

        statusBar()->showMessage("Radio station deleted.", 3000);
    }
}

void MainWindow::onRadioClearClicked()
{
    // Clear all input fields
    m_radioNameEdit->clear();
    m_radioStreamUrlEdit->clear();
    m_radioCountryEdit->clear();
    m_radioBitrateEdit->clear();
    m_radioCodecEdit->clear();
    m_radioCommentsEdit->clear();

    // Clear the icon display
    m_radioIconLabel->clear();
    m_radioIconLabel->setText("No Icon");

    // Reset the selected index (no station selected)
    m_currentRadioIndex = -1;

    // Remove highlight from all cards
    for (auto card : m_radioCards) {
        highlightCard(card, false);
    }

    // Clear any status messages
    statusBar()->showMessage("Form cleared.", 2000);
}

void MainWindow::onRadioAddClicked()
{
    // Get data from the detail panel line edits
    QString name = m_radioNameEdit->text().trimmed();
    QString streamUrl = m_radioStreamUrlEdit->text().trimmed();
    QString country = m_radioCountryEdit->text().trimmed();
    QString genre = m_radioGenreEdit->text().trimmed();
    int bitrate = m_radioBitrateEdit->text().trimmed().toInt();
    QString codec = m_radioCodecEdit->text().trimmed();
    QString comments = m_radioCommentsEdit->toPlainText().trimmed();

    // Validate required fields
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Please enter a station name.");
        return;
    }
    if (streamUrl.isEmpty()) {
        QMessageBox::warning(this, "Invalid URL", "Please enter a stream URL.");
        return;
    }

    // Create new station
    RadioStation newStation;
    newStation.stationuuid = QUuid::createUuid().toString();
    newStation.name = name;
    newStation.streamUrl = streamUrl;
    newStation.countrycode = country;
    newStation.genre = genre;
    newStation.bitrate = bitrate;
    newStation.codec = codec;
    newStation.comments = comments;
    newStation.votes = 0;
    newStation.isPlaying = false;
    newStation.iconUrl = "";  // Will be set later if needed

    // Add to vector
    m_radioStations.append(newStation);

    // Save to disk
    saveRadioStations();

    // Update the grid (this will call createRadioCard for ALL stations, including the new one)
    updateRadioGrid();

    // Clear the form
    onRadioClearClicked();

    // Select the newly added station
    onRadioStationSelected(m_radioStations.size() - 1);

    statusBar()->showMessage("Radio station added successfully.", 3000);
}

void MainWindow::onRadioPlayClicked() {
    if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {

        for (QFrame* card : m_radioCards) {
            card->setProperty("playing", false);
            card->style()->unpolish(card);
            card->style()->polish(card);
        }

        const RadioStation& station = m_radioStations[m_currentRadioIndex];
        if (!station.streamUrl.isEmpty()) {
            showStreamLoadingDialog(7000);
            player->setUrl(station.streamUrl);
            player->setWindowTitle(QString("Playing: %1").arg(station.name));
            player->play();


            // Add playing property to current card
            if (m_radioCards.contains(m_currentRadioIndex)) {
                m_radioCards[m_currentRadioIndex]->setProperty("playing", true);
                m_radioCards[m_currentRadioIndex]->style()->unpolish(m_radioCards[m_currentRadioIndex]);
                m_radioCards[m_currentRadioIndex]->style()->polish(m_radioCards[m_currentRadioIndex]);
            }

            player->show();
            statusBar()->showMessage(QString("Now Playing: %1 - %2").arg(station.name), 0);
            m_radioPlayButton->setEnabled(false);
            m_radioStopButton->setEnabled(true);
        }
    }
}

void MainWindow::onRadioStopClicked() {
    player->stop();  // Now public
    m_radioPlayButton->setEnabled(true);
    m_radioStopButton->setEnabled(false);
    statusBar()->showMessage(QString());

    for (QFrame* card : m_radioCards) {
        card->setProperty("playing", false);
        card->style()->unpolish(card);
        card->style()->polish(card);
    }

}


void MainWindow::onRadioStationSelected(int index) {
    if (index < 0 || index >= m_radioStations.size()) return;

    m_currentRadioIndex = index;
    const RadioStation& station = m_radioStations[index];

    m_radioNameEdit->setText(station.name);
    m_radioStreamUrlEdit->setText(station.streamUrl);
    m_radioCountryEdit->setText(station.countrycode);
    m_radioGenreEdit->setText(station.genre);
    m_radioBitrateEdit->setText(QString::number(station.bitrate));
    m_radioCodecEdit->setText(station.codec);
    m_radioCommentsEdit->setPlainText(station.comments);


    // Load icon from local path
    if (!station.iconUrl.isEmpty() && QFile::exists(station.iconUrl)) {
        QPixmap pixmap(station.iconUrl);
        if (!pixmap.isNull()) {
            m_radioIconLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_radioIconLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(64, 64));
        }
    } else {
        m_radioIconLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(64, 64));
    }
    // Highlight the selected card (like website does)
    for (int i = 0; i < m_radioCards.size(); ++i) {
        highlightCard(m_radioCards[i], i == index);
    }
}

void MainWindow::updateRadioGrid()
{
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_radioGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_radioCards.clear();

    // If no stations, show a message
    if (m_radioStations.isEmpty()) {
        QLabel* emptyLabel = new QLabel("No radio stations added yet.\n\nUse Tools → Browse Radio Stations to add some.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setWordWrap(true);
        emptyLabel->setMinimumHeight(400);
        emptyLabel->setObjectName("emptyMessageLabel");
        m_radioGrid->addWidget(emptyLabel, 0, 0, 1, 3);
        return;
    }

    // Add radio cards
    int row = 0;
    int col = 0;
    int maxCols = 3;

    for (int i = 0; i < m_radioStations.size(); ++i) {
        QFrame* card = createRadioCard(m_radioStations[i], i);
        m_radioGrid->addWidget(card, row, col);
        m_radioCards[i] = card;

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    // Add stretch at the end
    m_radioGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);

    // Restore selection if there was one
    if (m_currentRadioIndex >= 0 && m_currentRadioIndex < m_radioStations.size()) {
        // Card map has been rebuilt, need to re-highlight
        for (int i = 0; i < m_radioCards.size(); ++i) {
            highlightCard(m_radioCards[i], i == m_currentRadioIndex);
        }
    } else if (!m_radioStations.isEmpty()) {
        // No selection, select first
        onRadioStationSelected(0);
    }
}

QFrame* MainWindow::createRadioCard(const RadioStation& station, int index)
{
    QFrame* card = new QFrame();
    card->setObjectName(QString("radioCard_%1").arg(index));
    card->setProperty("radioIndex", index);
    card->setFrameShape(QFrame::StyledPanel);
    card->setObjectName("radioCard");
    card->setFixedSize(200, 200);
    card->setCursor(Qt::ArrowCursor);
    // Add shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15);

    // Create a horizontal layout for title and icon
    QHBoxLayout* titleLayout = new QHBoxLayout();

    // Add icon (default radio icon for now)
    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(16, 16);
    //iconLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(16, 16));

    if (!station.iconUrl.isEmpty() && QFile::exists(station.iconUrl)) {
        QPixmap pixmap(station.iconUrl);
        if (!pixmap.isNull()) {
            iconLabel->setPixmap(pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(16, 16));
        }
    } else {
        iconLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(16, 16));
    }

    titleLayout->addWidget(iconLabel);

    // Title (station name)
    QLabel* titleLabel = new QLabel(station.name);
    titleLabel->setObjectName("cardTitle");
    titleLabel->setWordWrap(true);

    titleLayout->addWidget(titleLabel, 1);

    layout->addLayout(titleLayout);

    // Info line (country + bitrate)
    QLabel* infoLabel = new QLabel(QString("%1 · %2").arg(station.countrycode, station.genre));
    infoLabel->setObjectName("cardInfo");
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Add action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    // Play button
    QPushButton* playBtn = new QPushButton();
    playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playBtn->setToolTip("Play Station");
    playBtn->setFlat(true);
    playBtn->setCursor(Qt::PointingHandCursor);
    playBtn->setObjectName("cardButton");

    // Stop button
    QPushButton* stopBtn = new QPushButton();
    stopBtn->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopBtn->setToolTip("Stop Station");
    stopBtn->setFlat(true);
    stopBtn->setCursor(Qt::PointingHandCursor);
    stopBtn->setObjectName("cardButton");

    // Delete button
    QPushButton* deleteBtn = new QPushButton();
    deleteBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    deleteBtn->setToolTip("Delete Station");
    deleteBtn->setFlat(true);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setObjectName("cardButton");

    buttonLayout->addWidget(playBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    // Connect button signals
    connect(playBtn, &QPushButton::clicked, [this, index]() {
        onRadioStationSelected(index);
        onRadioPlayClicked();
    });

    connect(stopBtn, &QPushButton::clicked, [this, index]() {
        onRadioStationSelected(index);
        onRadioStopClicked();
    });

    connect(deleteBtn, &QPushButton::clicked, [this, index]() {
        if (index >= 0 && index < m_radioStations.size()) {
            m_radioStations.removeAt(index);
            saveRadioStations();
            updateRadioGrid();
            if (m_radioStations.isEmpty()) {
                // Clear detail panel
                m_radioNameEdit->clear();
                m_radioStreamUrlEdit->clear();
                m_radioCountryEdit->clear();
                m_radioBitrateEdit->clear();
                m_radioCodecEdit->clear();
                m_radioCommentsEdit->clear();
                m_radioIconLabel->clear();
            }
        }
    });

    // Connect card click (for selection without playing)
    card->installEventFilter(this);
    card->setProperty("radioIndex", index);

    return card;
}


QString MainWindow::getRadioStationsFilePath()
{
    return JASMINE_CONSTANTS::appDirPath + "/radiostations.dat";
}

void MainWindow::saveRadioStations()
{
    QString filePath = getRadioStationsFilePath();
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);

        // Write the count first
        out << quint32(m_radioStations.size());

        // Write each station individually
        for (const RadioStation &station : m_radioStations) {
            out << station.stationuuid;
            out << station.name;
            out << station.streamUrl;
            out << station.iconUrl;
            out << station.countrycode;
            out << station.genre;
            out << station.bitrate;
            out << station.codec;
            out << station.votes;
            out << station.comments;
            out << station.isPlaying;
        }

        file.close();
    }
}

void MainWindow::loadRadioStations()
{
    QString filePath = getRadioStationsFilePath();
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);

        // Read the count
        quint32 count;
        in >> count;

        // Clear existing stations
        m_radioStations.clear();

        // Read each station
        for (quint32 i = 0; i < count; ++i) {
            RadioStation station;
            in >> station.stationuuid;
            in >> station.name;
            in >> station.streamUrl;
            in >> station.iconUrl;
            in >> station.countrycode;
            in >> station.genre;
            in >> station.bitrate;
            in >> station.codec;
            in >> station.votes;
            in >> station.comments;
            in >> station.isPlaying;
            m_radioStations.append(station);
        }

        file.close();
    }
    if (!m_radioStations.isEmpty()) {
        onRadioStationSelected(0);
    }
}

void MainWindow::onAddRadioStationFromDialog(const RadioStation &station)
{


    // Check if station already exists (by stationuuid)
    for (const auto &existing : m_radioStations) {
        if (existing.stationuuid == station.stationuuid) {
            statusBar()->showMessage("Station already exists in your collection.", 3000);
            return;
        }
    }

    // Add the station
    m_radioStations.append(station);

    // Save to disk
    saveRadioStations();

    // Update the grid
    updateRadioGrid();

    // Switch to radio tab (index 2)
    if (m_leftPanelTabs && m_leftPanelTabs->count() > 2) {
        m_leftPanelTabs->setCurrentIndex(2);
    }


    // Select the newly added station
    onRadioStationSelected(m_radioStations.size() - 1);

    statusBar()->showMessage(QString("Added: %1").arg(station.name), 3000);
}


/*
void MainWindow::searchCards(const QString& text)
{
    int currentTab = m_leftPanelTabs->currentIndex();

    if (currentTab == 0) {
        // Website cards
        for (int i = 0; i < m_websiteCards.size(); ++i) {
            QFrame* card = m_websiteCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
        }
    } else if (currentTab == 1) {
        // Session cards
        for (int i = 0; i < m_sessionCards.size(); ++i) {
            QFrame* card = m_sessionCards.values()[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
        }
    } else if (currentTab == 2) {
        // Radio cards
        for (int i = 0; i < m_radioCards.size(); ++i) {
            QFrame* card = m_radioCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
        }
    } else if (currentTab == 3) {
        // IPTV cards
        for (int i = 0; i < m_iptvCards.size(); ++i) {
            QFrame* card = m_iptvCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
        }
    }
}
*/

void MainWindow::searchCards(const QString& text)
{
    int currentTab = m_leftPanelTabs->currentIndex();

    if (currentTab == 0) {
        // Website cards
        QVector<bool> visible(m_websiteCards.size(), false);
        for (int i = 0; i < m_websiteCards.size(); ++i) {
            QFrame* card = m_websiteCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
            visible[i] = matches;
        }
        relayoutCards(m_websitesGrid, m_websiteCards, visible);

    } else if (currentTab == 1) {
        // Session cards
        QVector<bool> visible(m_sessionCards.size(), false);
        QList<QFrame*> sessionCardList = m_sessionCards.values();

        // Create a temporary map with indices
        QMap<int, QFrame*> tempMap;
        for (int i = 0; i < sessionCardList.size(); ++i) {
            tempMap[i] = sessionCardList[i];
        }

        for (int i = 0; i < sessionCardList.size(); ++i) {
            QFrame* card = sessionCardList[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
            visible[i] = matches;
        }
            relayoutCards(m_sessionsGrid, tempMap, visible);
    } else if (currentTab == 2) {
        // Radio cards
        QVector<bool> visible(m_radioCards.size(), false);
        for (int i = 0; i < m_radioCards.size(); ++i) {
            QFrame* card = m_radioCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
            visible[i] = matches;
        }
        relayoutCards(m_radioGrid, m_radioCards, visible);

    } else if (currentTab == 3) {
        // IPTV cards
        QVector<bool> visible(m_iptvCards.size(), false);
        for (int i = 0; i < m_iptvCards.size(); ++i) {
            QFrame* card = m_iptvCards[i];
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            bool matches = text.isEmpty() || (titleLabel && titleLabel->text().contains(text, Qt::CaseInsensitive));
            card->setVisible(matches);
            visible[i] = matches;
        }
        relayoutCards(m_iptvGrid, m_iptvCards, visible);
    }
}

void MainWindow::showStreamLoadingDialog(int duration)
{

    m_trayIcon->showMessage("Jasmine Internet, Radio and IPTV",
                         "Connecting to stream... Please wait.",
                         QSystemTrayIcon::Information,
                         duration);

    }

//////////////////////////// IPTV

void MainWindow::createIPTVTab()
{
    m_iptvScrollArea = new QScrollArea();
    m_iptvScrollArea->setWidgetResizable(true);
    m_iptvScrollArea->setFrameShape(QFrame::NoFrame);
    m_iptvContainer = new QWidget();
    m_iptvGrid = new QGridLayout(m_iptvContainer);
    m_iptvGrid->setSpacing(12);
    m_iptvGrid->setContentsMargins(16, 16, 16, 16);
    m_iptvGrid->setSizeConstraint(QLayout::SetFixedSize);
    for (int i = 0; i < 3; ++i) {
        m_iptvGrid->setColumnStretch(i, 0);
    }
    m_iptvScrollArea->setWidget(m_iptvContainer);

    m_leftPanelTabs->addTab(m_iptvScrollArea, createRotatedIcon(":/resources/icons/tv.svg"), QString());
    m_leftPanelTabs->tabBar()->setTabToolTip(3, "IPTV");

    QFrame* iptvDetailsPanel = createIPTVDetailPanel();
    iptvDetailsPanel->setMinimumWidth(200);
    detailsStack->addWidget(iptvDetailsPanel);

    loadIPTVChannels();

    if (!m_iptvChannels.isEmpty()) {
        updateIPTVGrid();
        onIPTVStationSelected(0);
    }
}

QFrame* MainWindow::createIPTVDetailPanel()
{
    QFrame* panel = new QFrame();
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setObjectName("iptvDetailsPanel");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);

    QLabel* titleLabel = new QLabel("IPTV Channel");
    titleLabel->setObjectName("detailsPanelTitle");
    layout->addWidget(titleLabel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Icon with generate button
    QHBoxLayout* iconLayout = new QHBoxLayout();
    m_iptvIconLabel = new QLabel();
    m_iptvIconLabel->setFixedSize(64, 64);
    m_iptvIconLabel->setAlignment(Qt::AlignCenter);
    m_iptvIconLabel->setFrameShape(QFrame::StyledPanel);
    m_iptvIconLabel->setObjectName("iptvIconPreview");
    m_iptvIconLabel->setScaledContents(true);

    m_iptvGenerateIconButton = new QPushButton();
    m_iptvGenerateIconButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_iptvGenerateIconButton->setFixedSize(24, 24);
    m_iptvGenerateIconButton->setToolTip("Generate icon from channel name");

    iconLayout->addWidget(m_iptvIconLabel);
    iconLayout->addWidget(m_iptvGenerateIconButton);
    iconLayout->addStretch();
    formLayout->addRow("Icon:", iconLayout);

    // Editable fields
    m_iptvNameEdit = new QLineEdit();
    m_iptvNameEdit->setPlaceholderText("Channel name");
    formLayout->addRow("Name:", m_iptvNameEdit);

    m_iptvStreamUrlEdit = new QLineEdit();
    m_iptvStreamUrlEdit->setPlaceholderText("Stream URL (.m3u8)");
    formLayout->addRow("Stream URL:", m_iptvStreamUrlEdit);

    m_iptvCategoryEdit = new QLineEdit();
    m_iptvCategoryEdit->setPlaceholderText("Category (News, Sports, etc.)");
    formLayout->addRow("Category:", m_iptvCategoryEdit);

    m_iptvCountryEdit = new QLineEdit();
    m_iptvCountryEdit->setPlaceholderText("Country code (e.g., US, GR)");
    formLayout->addRow("Country:", m_iptvCountryEdit);

    m_iptvCommentsEdit = new QTextEdit();
    m_iptvCommentsEdit->setPlaceholderText("Optional comments");
    m_iptvCommentsEdit->setMinimumHeight(80);
    formLayout->addRow("Comments:", m_iptvCommentsEdit);

    layout->addLayout(formLayout);

    // Buttons row 1: Play, Stop
    QHBoxLayout* buttonLayout1 = new QHBoxLayout();
    buttonLayout1->setSpacing(8);
    m_iptvPlayButton = new QPushButton("Play");
    m_iptvPlayButton->setObjectName("primaryButton");
    m_iptvStopButton = new QPushButton("Stop");
    m_iptvStopButton->setObjectName("secondaryButton");
    m_iptvStopButton->setEnabled(false);
    buttonLayout1->addStretch();
    buttonLayout1->addWidget(m_iptvPlayButton);
    buttonLayout1->addWidget(m_iptvStopButton);
    buttonLayout1->addStretch();
    //layout->addLayout(buttonLayout1);

    // Buttons row 2: Add, Update, Delete, Clear
    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    buttonLayout2->setSpacing(8);
    m_iptvAddButton = new QPushButton("Add");
    m_iptvAddButton->setObjectName("primaryButton");
    m_iptvUpdateButton = new QPushButton("Update");
    m_iptvUpdateButton->setObjectName("secondaryButton");
    m_iptvDeleteButton = new QPushButton("Delete");
    m_iptvDeleteButton->setObjectName("secondaryButton");
    m_iptvClearButton = new QPushButton("Clear");
    m_iptvClearButton->setObjectName("secondaryButton");
    buttonLayout2->addStretch();
    buttonLayout2->addWidget(m_iptvAddButton);
    buttonLayout2->addWidget(m_iptvUpdateButton);
    buttonLayout2->addWidget(m_iptvDeleteButton);
    buttonLayout2->addWidget(m_iptvClearButton);
    buttonLayout2->addStretch();
    layout->addLayout(buttonLayout2);
    layout->addLayout(buttonLayout1);

    layout->addStretch();

    // Connect signals
    connect(m_iptvPlayButton, &QPushButton::clicked, this, &MainWindow::onIPTVPlayClicked);
    connect(m_iptvStopButton, &QPushButton::clicked, this, &MainWindow::onIPTVStopClicked);
    connect(m_iptvAddButton, &QPushButton::clicked, this, &MainWindow::onIPTVAddClicked);
    connect(m_iptvUpdateButton, &QPushButton::clicked, this, &MainWindow::onIPTVUpdateClicked);
    connect(m_iptvDeleteButton, &QPushButton::clicked, this, &MainWindow::onIPTVDeleteClicked);
    connect(m_iptvClearButton, &QPushButton::clicked, this, &MainWindow::onIPTVClearClicked);
    connect(m_iptvGenerateIconButton, &QPushButton::clicked, [this]() {
        if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {
            QIcon newIcon = generateRandomSvgIcon();
            QPixmap pixmap = newIcon.pixmap(64, 64);
            m_iptvIconLabel->setPixmap(pixmap);

            QString iconPath = JASMINE_CONSTANTS::iconDir + "/" +
                              m_iptvChannels[m_currentIPTVIndex].channelId + "_iptv.png";
            pixmap.save(iconPath);
            m_iptvChannels[m_currentIPTVIndex].localLogoPath = iconPath;
            saveIPTVChannels();
            updateIPTVGrid();
        }
    });

    return panel;
}

QFrame* MainWindow::createIPTVCard(const IPTVChannel& channel, int index)
{
    QFrame* card = new QFrame();
    card->setObjectName(QString("iptvCard_%1").arg(index));
    card->setProperty("iptvIndex", index);
    card->setFrameShape(QFrame::StyledPanel);
    card->setObjectName("iptvCard");
    card->setFixedSize(200, 200);
    card->setCursor(Qt::ArrowCursor);
    card->installEventFilter(this);

    // Shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15);

    // Title row with icon
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(16, 16);

    if (!channel.localLogoPath.isEmpty() && QFile::exists(channel.localLogoPath)) {
        QPixmap pixmap(channel.localLogoPath);
        if (!pixmap.isNull()) {
            iconLabel->setPixmap(pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(16, 16));
        }
    } else if (!channel.logoUrl.isEmpty()) {
        iconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(16, 16));
    } else {
        iconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(16, 16));
    }
    titleLayout->addWidget(iconLabel);

    // Channel name
    QLabel* titleLabel = new QLabel(channel.name);
    titleLabel->setObjectName("cardTitle");
    titleLabel->setWordWrap(true);
    titleLayout->addWidget(titleLabel, 1);
    layout->addLayout(titleLayout);

    // Info line (category + country)
    QString infoText = channel.category;
    if (!channel.country.isEmpty()) {
        infoText += (infoText.isEmpty() ? "" : " · ") + channel.country;
    }
    if (infoText.isEmpty()) infoText = "No category";
    QLabel* infoLabel = new QLabel(infoText);
    infoLabel->setObjectName("cardInfo");
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    QPushButton* playBtn = new QPushButton();
    playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playBtn->setToolTip("Play Channel");
    playBtn->setFlat(true);
    playBtn->setCursor(Qt::PointingHandCursor);
    playBtn->setObjectName("cardButton");

    QPushButton* stopBtn = new QPushButton();
    stopBtn->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopBtn->setToolTip("Stop Channel");
    stopBtn->setFlat(true);
    stopBtn->setCursor(Qt::PointingHandCursor);
    stopBtn->setObjectName("cardButton");

    QPushButton* deleteBtn = new QPushButton();
    deleteBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    deleteBtn->setToolTip("Delete Channel");
    deleteBtn->setFlat(true);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setObjectName("cardButton");

    buttonLayout->addWidget(playBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    // Connect signals
    connect(playBtn, &QPushButton::clicked, [this, index]() {
        onIPTVStationSelected(index);
        onIPTVPlayClicked();
    });

    connect(stopBtn, &QPushButton::clicked, [this, index]() {
        onIPTVStationSelected(index);
        onIPTVStopClicked();
    });

    connect(deleteBtn, &QPushButton::clicked, [this, index]() {
        onIPTVStationSelected(index);
        onIPTVDeleteClicked();
    });

    card->setProperty("iptvIndex", index);

    return card;
}

void MainWindow::updateIPTVGrid()
{
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_iptvGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_iptvCards.clear();

    // If no channels, show message
    if (m_iptvChannels.isEmpty()) {
        QLabel* emptyLabel = new QLabel("No IPTV channels added yet.\n\nUse Tools → Browse IPTV Channels to add some.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setWordWrap(true);
        emptyLabel->setMinimumHeight(400);
        emptyLabel->setObjectName("emptyMessageLabel");
        m_iptvGrid->addWidget(emptyLabel, 0, 0, 1, 3);
        return;
    }

    // Add cards
    int row = 0;
    int col = 0;
    int maxCols = 3;

    for (int i = 0; i < m_iptvChannels.size(); ++i) {
        QFrame* card = createIPTVCard(m_iptvChannels[i], i);
        m_iptvGrid->addWidget(card, row, col);
        m_iptvCards[i] = card;

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    // Add stretch
    m_iptvGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);

    // Restore selection
    if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {
        for (int i = 0; i < m_iptvCards.size(); ++i) {
            highlightCard(m_iptvCards[i], i == m_currentIPTVIndex);
        }
    } else if (!m_iptvChannels.isEmpty()) {
        onIPTVStationSelected(0);
    }
}

QString MainWindow::getIPTVChannelsFilePath()
{
    return JASMINE_CONSTANTS::appDirPath + "/iptvchannels.dat";
}

void MainWindow::saveIPTVChannels()
{
    QString filePath = getIPTVChannelsFilePath();
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint32(m_iptvChannels.size());

        for (const IPTVChannel& channel : m_iptvChannels) {
            out << channel.channelId;
            out << channel.name;
            out << channel.streamUrl;
            out << channel.logoUrl;
            out << channel.category;
            out << channel.country;
            out << channel.referrer;
            out << channel.userAgent;
            out << channel.comments;
            out << channel.localLogoPath;
        }
        file.close();
    }
}

void MainWindow::loadIPTVChannels()
{
    QString filePath = getIPTVChannelsFilePath();
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);

        quint32 count;
        in >> count;
        m_iptvChannels.clear();

        for (quint32 i = 0; i < count; ++i) {
            IPTVChannel channel;
            in >> channel.channelId;
            in >> channel.name;
            in >> channel.streamUrl;
            in >> channel.logoUrl;
            in >> channel.category;
            in >> channel.country;
            in >> channel.referrer;
            in >> channel.userAgent;
            in >> channel.comments;
            in >> channel.localLogoPath;
            channel.isPlaying = false;
            m_iptvChannels.append(channel);
        }
        file.close();
    }
}

////// slot functions
/*
void MainWindow::onIPTVStationSelected(int index)
{
    if (index < 0 || index >= m_iptvChannels.size()) return;

    m_currentIPTVIndex = index;
    const IPTVChannel& channel = m_iptvChannels[index];

    m_iptvNameEdit->setText(channel.name);
    m_iptvStreamUrlEdit->setText(channel.streamUrl);
    m_iptvCategoryEdit->setText(channel.category);
    m_iptvCountryEdit->setText(channel.country);
    m_iptvCommentsEdit->setPlainText(channel.comments);

    // Load icon
    if (!channel.localLogoPath.isEmpty() && QFile::exists(channel.localLogoPath)) {
        QPixmap pixmap(channel.localLogoPath);
        if (!pixmap.isNull()) {
            m_iptvIconLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_iptvIconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(64, 64));
        }
    } else {
        m_iptvIconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(64, 64));
    }

    // Highlight selected card
    for (int i = 0; i < m_iptvCards.size(); ++i) {
        highlightCard(m_iptvCards[i], i == index);
    }
}
*/

void MainWindow::onIPTVStationSelected(int index)
{
    if (index < 0 || index >= m_iptvChannels.size()) return;

    m_currentIPTVIndex = index;
    IPTVChannel& channel = m_iptvChannels[index];  // Note: non-const reference

    m_iptvNameEdit->setText(channel.name);
    m_iptvStreamUrlEdit->setText(channel.streamUrl);
    m_iptvCategoryEdit->setText(channel.category);
    m_iptvCountryEdit->setText(channel.country);
    m_iptvCommentsEdit->setPlainText(channel.comments);

    // Check if we have a valid local icon (either from download or user-generated)
    bool hasLocalIcon = !channel.localLogoPath.isEmpty() && QFile::exists(channel.localLogoPath);

    if (hasLocalIcon) {
        QPixmap pixmap(channel.localLogoPath);
        if (!pixmap.isNull()) {
            m_iptvIconLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // Local path exists but file is corrupted - use default
            m_iptvIconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(64, 64));
            // Only download if there's a URL and no user-generated icon exists
            if (!channel.logoUrl.isEmpty() && channel.localLogoPath.isEmpty()) {
                downloadIPTVIconIfNeeded(index);
            }
        }
    } else {
        // No local icon - show default
        m_iptvIconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(64, 64));

        // Only download if there's a logoUrl AND user hasn't generated a custom icon
        // (localLogoPath empty means no user-generated icon yet)
        if (!channel.logoUrl.isEmpty() && channel.localLogoPath.isEmpty()) {
            downloadIPTVIconIfNeeded(index);
        }
    }

    // Highlight selected card
    for (int i = 0; i < m_iptvCards.size(); ++i) {
        highlightCard(m_iptvCards[i], i == index);
    }
}

void MainWindow::onIPTVAddClicked()
{
    QString name = m_iptvNameEdit->text().trimmed();
    QString streamUrl = m_iptvStreamUrlEdit->text().trimmed();
    QString category = m_iptvCategoryEdit->text().trimmed();
    QString country = m_iptvCountryEdit->text().trimmed();
    QString comments = m_iptvCommentsEdit->toPlainText().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Please enter a channel name.");
        return;
    }
    if (streamUrl.isEmpty()) {
        QMessageBox::warning(this, "Invalid URL", "Please enter a stream URL.");
        return;
    }

    IPTVChannel newChannel;
    newChannel.channelId = QUuid::createUuid().toString();
    newChannel.name = name;
    newChannel.streamUrl = streamUrl;
    newChannel.category = category;
    newChannel.country = country;
    newChannel.comments = comments;
    newChannel.isPlaying = false;

    m_iptvChannels.append(newChannel);
    saveIPTVChannels();
    updateIPTVGrid();
    onIPTVClearClicked();
    onIPTVStationSelected(m_iptvChannels.size() - 1);
    statusBar()->showMessage("IPTV channel added successfully.", 3000);
}

void MainWindow::onIPTVUpdateClicked()
{
    if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {
        m_iptvChannels[m_currentIPTVIndex].name = m_iptvNameEdit->text().trimmed();
        m_iptvChannels[m_currentIPTVIndex].streamUrl = m_iptvStreamUrlEdit->text().trimmed();
        m_iptvChannels[m_currentIPTVIndex].category = m_iptvCategoryEdit->text().trimmed();
        m_iptvChannels[m_currentIPTVIndex].country = m_iptvCountryEdit->text().trimmed();
        m_iptvChannels[m_currentIPTVIndex].comments = m_iptvCommentsEdit->toPlainText().trimmed();

        saveIPTVChannels();
        updateIPTVGrid();
        onIPTVStationSelected(m_currentIPTVIndex);
        statusBar()->showMessage("IPTV channel updated.", 3000);
    }
}

void MainWindow::onIPTVDeleteClicked()
{
    if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {
        // Delete local icon file
        QString iconPath = m_iptvChannels[m_currentIPTVIndex].localLogoPath;
        if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
            QFile::remove(iconPath);
        }

        m_iptvChannels.removeAt(m_currentIPTVIndex);
        saveIPTVChannels();
        updateIPTVGrid();

        if (m_iptvChannels.isEmpty()) {
            onIPTVClearClicked();
            m_currentIPTVIndex = -1;
        } else {
            onIPTVStationSelected(0);
        }
        statusBar()->showMessage("IPTV channel deleted.", 3000);
    }
}

void MainWindow::onIPTVClearClicked()
{
    m_iptvNameEdit->clear();
    m_iptvStreamUrlEdit->clear();
    m_iptvCategoryEdit->clear();
    m_iptvCountryEdit->clear();
    m_iptvCommentsEdit->clear();
    m_iptvIconLabel->setPixmap(QIcon::fromTheme("video-x-generic").pixmap(64, 64));
    m_currentIPTVIndex = -1;

    for (auto card : m_iptvCards) {
        highlightCard(card, false);
    }
    statusBar()->showMessage("Form cleared.", 2000);
}

void MainWindow::onIPTVPlayClicked()
{
    if (m_currentIPTVIndex >= 0 && m_currentIPTVIndex < m_iptvChannels.size()) {

        for (QFrame* card : m_iptvCards) {
                    card->setProperty("playing", false);
                    card->style()->unpolish(card);
                    card->style()->polish(card);
                }

        IPTVChannel& channel = m_iptvChannels[m_currentIPTVIndex];
        if (!channel.streamUrl.isEmpty()) {
            showStreamLoadingDialog(7000);
            player->setUrl(channel.streamUrl);
            player->setWindowTitle(QString("Playing: %1").arg(channel.name));
            player->play();

            // Add playing property to current card ONLY
            if (m_iptvCards.contains(m_currentIPTVIndex)) {
                m_iptvCards[m_currentIPTVIndex]->setProperty("playing", true);
                m_iptvCards[m_currentIPTVIndex]->style()->unpolish(m_iptvCards[m_currentIPTVIndex]);
                m_iptvCards[m_currentIPTVIndex]->style()->polish(m_iptvCards[m_currentIPTVIndex]);
            }

            player->show();
            statusBar()->showMessage(QString("Now Playing: %1 - %2").arg(channel.name, channel.category), 0);
            m_iptvPlayButton->setEnabled(false);
            m_iptvStopButton->setEnabled(true);

        }
    }
}

void MainWindow::onIPTVStopClicked()
{
    player->stop();
    m_iptvPlayButton->setEnabled(true);
    m_iptvStopButton->setEnabled(false);
    statusBar()->showMessage(QString());

    for (QFrame* card : m_iptvCards) {
        card->setProperty("playing", false);
        card->style()->unpolish(card);
        card->style()->polish(card);
    }

}

void MainWindow::onAddIPTVChannelsFromDialog(const QVector<IPTVChannel>& channels)
{
    int addedCount = 0;
    for (const IPTVChannel& channel : channels) {
        // Check for duplicates by name
        bool exists = false;
        for (const auto& existing : m_iptvChannels) {
            if (existing.name == channel.name && existing.streamUrl == channel.streamUrl) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_iptvChannels.append(channel);
            addedCount++;
        }
    }

    if (addedCount > 0) {
        saveIPTVChannels();
        updateIPTVGrid();
        if (m_leftPanelTabs->count() > 3) {
            m_leftPanelTabs->setCurrentIndex(3);
        }
        onIPTVStationSelected(m_iptvChannels.size() - 1);
        statusBar()->showMessage(QString("Added %1 IPTV channel(s).").arg(addedCount), 3000);
    }
}

void MainWindow::relayoutCards(QGridLayout* grid, const QMap<int, QFrame*>& cards, const QVector<bool>& visible)
{
    // Remove all widgets from grid
    QLayoutItem* item;
    while ((item = grid->takeAt(0)) != nullptr) {
        // Don't delete widgets, just remove from layout
        delete item;
    }

    // Add back only visible cards in order
    int row = 0;
    int col = 0;
    int maxCols = 3;

    for (int i = 0; i < cards.size(); ++i) {
        if (visible[i]) {
            grid->addWidget(cards[i], row, col);
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }

    // Add stretch at the end
    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);
}

void MainWindow::downloadIPTVIconIfNeeded(int index)
{
    if (index < 0 || index >= m_iptvChannels.size()) return;

    IPTVChannel& channel = m_iptvChannels[index];

    // Don't download if there's already a local icon
    if (!channel.localLogoPath.isEmpty() && QFile::exists(channel.localLogoPath)) {
        return;
    }

    // Don't download if no URL to download from
    if (channel.logoUrl.isEmpty()) {
        return;
    }

    // Download the icon
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(channel.logoUrl)));

    connect(reply, &QNetworkReply::finished, [this, reply, nam, index]() {


        if (reply->error() == QNetworkReply::NoError) {
            QString iconPath = JASMINE_CONSTANTS::iconDir + "/iptv_" +
                              m_iptvChannels[index].channelId + ".png";

            QFile file(iconPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();

                m_iptvChannels[index].localLogoPath = iconPath;
                saveIPTVChannels();

                // Update UI if this channel is still selected
                if (m_currentIPTVIndex == index) {
                    QPixmap pixmap(iconPath);
                    if (!pixmap.isNull()) {
                        m_iptvIconLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                }

                // Update the card
                if (m_iptvCards.contains(index)) {
                    QLabel* iconLabel = m_iptvCards[index]->findChild<QLabel*>();
                    if (iconLabel) {
                        QPixmap pixmap(iconPath);
                        if (!pixmap.isNull()) {
                            iconLabel->setPixmap(pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        }
                    }
                }
            }
        }
        reply->deleteLater();
        nam->deleteLater();
    });
}

void MainWindow::sortAllCards()
{
    // Helper lambda to sort cards by title
    auto sortCards = [](QMap<int, QFrame*> cards, QGridLayout* grid) {
        // Collect card data with their titles
        QVector<QPair<QString, QFrame*>> cardList;
        for (auto it = cards.begin(); it != cards.end(); ++it) {
            QFrame* card = it.value();
            QLabel* titleLabel = card->findChild<QLabel*>("cardTitle");
            QString title = titleLabel ? titleLabel->text() : "";
            cardList.append({title, card});
        }

        // Sort alphabetically by title
        std::sort(cardList.begin(), cardList.end(),
            [](const QPair<QString, QFrame*>& a, const QPair<QString, QFrame*>& b) {
                return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
            });

        // Remove all widgets from grid
        QLayoutItem* item;
        while ((item = grid->takeAt(0)) != nullptr) {
            delete item;
        }

        // Add cards back in sorted order
        int row = 0;
        int col = 0;
        int maxCols = 3;
        for (const auto& pair : cardList) {
            grid->addWidget(pair.second, row, col);
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }

        // Add stretch at the end
        grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);
    };

    // Sort website cards
    if (!m_websiteCards.isEmpty()) {
        sortCards(m_websiteCards, m_websitesGrid);
    }

    /*
    // Sort session cards (convert QMap<QString, QFrame*> to indexed map)
    if (!m_sessionCards.isEmpty()) {
        QMap<int, QFrame*> indexedSessionCards;
        QList<QFrame*> sessionList = m_sessionCards.values();
        for (int i = 0; i < sessionList.size(); ++i) {
            indexedSessionCards[i] = sessionList[i];
        }
        sortCards(indexedSessionCards, m_sessionsGrid);

        // Rebuild m_sessionCards with sorted order (optional)
        QMap<QString, QFrame*> newSessionCards;
        for (int i = 0; i < sessionList.size(); ++i) {
            QFrame* card = sessionList[i];
            QString name = card->property("sessionName").toString();
            newSessionCards[name] = card;
        }
        m_sessionCards = newSessionCards;
    }
    */


    // Sort radio cards
    if (!m_radioCards.isEmpty()) {
        sortCards(m_radioCards, m_radioGrid);
    }

    // Sort IPTV cards
    if (!m_iptvCards.isEmpty()) {
        sortCards(m_iptvCards, m_iptvGrid);
    }
}

//////////////////////////// podcasts

void MainWindow::createPodcastTab()
{
    m_podcastScrollArea = new QScrollArea();
    m_podcastScrollArea->setWidgetResizable(true);
    m_podcastScrollArea->setFrameShape(QFrame::NoFrame);
    m_podcastContainer = new QWidget();
    m_podcastGrid = new QGridLayout(m_podcastContainer);
    m_podcastGrid->setSpacing(12);
    m_podcastGrid->setContentsMargins(16, 16, 16, 16);
    m_podcastGrid->setSizeConstraint(QLayout::SetFixedSize);
    for (int i = 0; i < 3; ++i) {
        m_podcastGrid->setColumnStretch(i, 0);
    }
    m_podcastScrollArea->setWidget(m_podcastContainer);

    m_leftPanelTabs->addTab(m_podcastScrollArea, createRotatedIcon(":/resources/icons/rss.svg"), QString());
    m_leftPanelTabs->tabBar()->setTabToolTip(4, "Podcasts");

    QFrame* podcastDetailsPanel = createPodcastDetailPanel();
    podcastDetailsPanel->setMinimumWidth(200);
    detailsStack->addWidget(podcastDetailsPanel);

    loadPodcasts();

    if (!m_podcastShows.isEmpty()) {
        updatePodcastGrid();
        onPodcastShowSelected(0);
    }
}

QFrame* MainWindow::createPodcastCard(const PodcastShow& show, int index)
{
    QFrame* card = new QFrame();
    card->setObjectName(QString("podcastCard_%1").arg(index));
    card->setProperty("podcastIndex", index);
    card->setFrameShape(QFrame::StyledPanel);
    card->setObjectName("podcastCard");
    card->setFixedSize(200, 200);
    card->setCursor(Qt::ArrowCursor);
    card->installEventFilter(this);

    // Shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(15, 15, 15, 15);

    // Artwork
    QLabel* artworkLabel = new QLabel();
    artworkLabel->setFixedSize(60, 60);
    artworkLabel->setAlignment(Qt::AlignCenter);
    artworkLabel->setScaledContents(true);

    if (!show.localArtworkPath.isEmpty() && QFile::exists(show.localArtworkPath)) {
        QPixmap pixmap(show.localArtworkPath);
        if (!pixmap.isNull()) {
            artworkLabel->setPixmap(pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            artworkLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(60, 60));
        }
    } else {
        artworkLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(60, 60));
    }
    layout->addWidget(artworkLabel, 0, Qt::AlignCenter);

    // Title
    QLabel* titleLabel = new QLabel(show.title);
    titleLabel->setObjectName("cardTitle");
    titleLabel->setWordWrap(false);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Author + episode count
    QLabel* infoLabel = new QLabel(QString("%1 · %2 eps").arg(show.author, QString::number(show.episodeCount)));
    infoLabel->setObjectName("cardInfo");
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);

    QPushButton* viewBtn = new QPushButton("Episodes");
    viewBtn->setToolTip("View episodes");
    viewBtn->setFlat(true);
    viewBtn->setObjectName("cardButton");
    //viewBtn->setVisible(false);
    QPushButton* unsubscribeBtn = new QPushButton("Unsub");
    unsubscribeBtn->setToolTip("Unsubscribe");
    unsubscribeBtn->setFlat(true);
    unsubscribeBtn->setObjectName("cardButton");

    //buttonLayout->addWidget(viewBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(unsubscribeBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    connect(viewBtn, &QPushButton::clicked, [this, index]() {
        onPodcastShowSelected(index);
    });

    connect(unsubscribeBtn, &QPushButton::clicked, [this, index]() {
        onPodcastShowSelected(index);
        onPodcastUnsubscribeClicked();
    });

    return card;
}

QFrame* MainWindow::createPodcastDetailPanel()
{
    QFrame* panel = new QFrame();
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setObjectName("podcastDetailsPanel");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);

    // ========== ARTWORK SECTION ==========

    /*
    QHBoxLayout* artworkLayout = new QHBoxLayout();
    artworkLayout->addStretch();
    m_podcastArtworkLabel = new QLabel();
    m_podcastArtworkLabel->setFixedSize(120, 120);
    m_podcastArtworkLabel->setAlignment(Qt::AlignCenter);
    m_podcastArtworkLabel->setScaledContents(true);
    m_podcastArtworkLabel->setFrameShape(QFrame::StyledPanel);
    m_podcastArtworkLabel->setObjectName("podcastArtworkPreview");
    artworkLayout->addWidget(m_podcastArtworkLabel);
    artworkLayout->addStretch();
    layout->addLayout(artworkLayout);

    */

    // In createPodcastDetailPanel(), replace the artwork section:

    // Artwork with generate button
    QHBoxLayout* artworkLayout = new QHBoxLayout();
    artworkLayout->addStretch();

    m_podcastArtworkLabel = new QLabel();
    m_podcastArtworkLabel->setFixedSize(100, 100);
    m_podcastArtworkLabel->setAlignment(Qt::AlignCenter);
    m_podcastArtworkLabel->setScaledContents(true);
    m_podcastArtworkLabel->setFrameShape(QFrame::StyledPanel);
    m_podcastArtworkLabel->setObjectName("podcastArtworkPreview");

    QPushButton* generateIconButton = new QPushButton();
    generateIconButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    generateIconButton->setFixedSize(24, 24);
    generateIconButton->setToolTip("Generate custom icon (replaces original artwork)");

    artworkLayout->addWidget(m_podcastArtworkLabel);
    artworkLayout->addWidget(generateIconButton);
    artworkLayout->addStretch();

    layout->addLayout(artworkLayout);

    // Connect the button
    connect(generateIconButton, &QPushButton::clicked, [this]() {
        if (m_currentPodcastIndex >= 0 && m_currentPodcastIndex < m_podcastShows.size()) {
            QMessageBox::StandardButton reply = QMessageBox::warning(
                this,
                "Generate Custom Icon",
                "This will replace the podcast artwork with a generated icon.\n\n"
                "Original artwork will be lost.\n"
                "To restore the original artwork, you will need to unsubscribe and subscribe again.\n\n"
                "Continue?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply != QMessageBox::Yes) {
                return;
            }

            QIcon newIcon = generateRandomSvgIcon();
            QPixmap pixmap = newIcon.pixmap(100, 100);
            m_podcastArtworkLabel->setPixmap(pixmap);

            QString iconPath = JASMINE_CONSTANTS::iconDir + "/podcast_" +
                              m_podcastShows[m_currentPodcastIndex].showId + "_generated.png";
            pixmap.save(iconPath);
            m_podcastShows[m_currentPodcastIndex].localArtworkPath = iconPath;
            savePodcasts();
            updatePodcastGrid();
        }
    });

    layout->addSpacing(8);

    // ========== PODCAST INFORMATION SECTION ==========
    QLabel* infoTitle = new QLabel("Podcast Information");
    infoTitle->setObjectName("detailsPanelSubtitle");
    //layout->addWidget(infoTitle);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Title
    m_podcastTitleLabel = new QLabel();
    m_podcastTitleLabel->setWordWrap(true);
    formLayout->addRow("Title:", m_podcastTitleLabel);

    // Author / Publisher
    m_podcastAuthorLabel = new QLabel();
    m_podcastAuthorLabel->setWordWrap(true);
    formLayout->addRow("Author:", m_podcastAuthorLabel);

    // Category
    m_podcastCategoryLabel = new QLabel();
    m_podcastCategoryLabel->setWordWrap(true);
    formLayout->addRow("Category:", m_podcastCategoryLabel);

    // Feed URL
    m_podcastFeedUrlLabel = new QLabel();
    m_podcastFeedUrlLabel->setWordWrap(true);
    m_podcastFeedUrlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    formLayout->addRow("Feed URL:", m_podcastFeedUrlLabel);

    // Website URL
    m_podcastWebsiteLabel = new QLabel();
    m_podcastWebsiteLabel->setWordWrap(true);
    m_podcastWebsiteLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_podcastWebsiteLabel->setOpenExternalLinks(true);
    formLayout->addRow("Website:", m_podcastWebsiteLabel);

    // Episode count & Last updated
    QHBoxLayout* statsLayout = new QHBoxLayout();
    m_podcastEpisodeCountLabel = new QLabel();
    m_podcastLastUpdatedLabel = new QLabel();
    statsLayout->addWidget(m_podcastEpisodeCountLabel);
    statsLayout->addWidget(m_podcastLastUpdatedLabel);
    statsLayout->addStretch();
    formLayout->addRow("Stats:", statsLayout);

    // Description
    m_podcastDescriptionLabel = new QLabel();
    m_podcastDescriptionLabel->setWordWrap(true);
    m_podcastDescriptionLabel->setMinimumHeight(60);
    m_podcastDescriptionLabel->setAlignment(Qt::AlignTop);
    //formLayout->addRow("Description:", m_podcastDescriptionLabel);

    // Comments section with toggle button
    QVBoxLayout* commentsLayout = new QVBoxLayout();

    // Button row
    QHBoxLayout* buttonRow = new QHBoxLayout();
    //buttonRow->addStretch();

    QPushButton* commentsToggleButton = new QPushButton("Show Notes");
    commentsToggleButton->setCheckable(true);
    commentsToggleButton->setChecked(false);
    commentsToggleButton->setObjectName("secondaryButton");
    commentsToggleButton->setMaximumHeight(30);
    buttonRow->addWidget(commentsToggleButton);
    commentsLayout->addLayout(buttonRow);

    // Comments text edit (initially invisible)
    m_podcastCommentsEdit = new QTextEdit();
    m_podcastCommentsEdit->setPlaceholderText("Add your personal notes about this podcast...");
    m_podcastCommentsEdit->setMaximumHeight(100);
    m_podcastCommentsEdit->setVisible(false);
    commentsLayout->addWidget(m_podcastCommentsEdit);
    formLayout->addRow(commentsLayout);
    // Connect toggle button
    connect(commentsToggleButton, &QPushButton::toggled, [this, commentsToggleButton](bool checked) {
        m_podcastCommentsEdit->setVisible(checked);
        commentsToggleButton->setText(checked ? "Hide Notes" : "Show Notes");
    });

    // Auto-save comments
    connect(m_podcastCommentsEdit, &QTextEdit::textChanged, [this]() {
        if (m_currentPodcastIndex >= 0 && m_currentPodcastIndex < m_podcastShows.size()) {
            m_podcastShows[m_currentPodcastIndex].comments = m_podcastCommentsEdit->toPlainText();
            savePodcasts();
        }
    });
    layout->addLayout(formLayout);

    layout->addSpacing(8);

    // ========== EPISODE LIST SECTION ==========
    QLabel* episodesLabel = new QLabel("Episodes");
    episodesLabel->setObjectName("detailsPanelSubtitle");
    layout->addWidget(episodesLabel);

    m_podcastEpisodeList = new QListWidget();
    m_podcastEpisodeList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_podcastEpisodeList->setAlternatingRowColors(true);
    layout->addWidget(m_podcastEpisodeList);

    // ========== PLAYBACK CONTROLS SECTION ==========
    QLabel* playbackTitle = new QLabel("Playback");
    playbackTitle->setObjectName("detailsPanelSubtitle");
    layout->addWidget(playbackTitle);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_podcastRefreshButton = new QPushButton("Refresh Feed");
    m_podcastRefreshButton->setToolTip("Check for new episodes");
    m_podcastRefreshButton->setObjectName("secondaryButton");

    m_podcastUnsubscribeButton = new QPushButton("Unsubscribe");
    m_podcastUnsubscribeButton->setToolTip("Remove podcast from your collection");
    m_podcastUnsubscribeButton->setObjectName("secondaryButton");

    m_podcastPlayButton = new QPushButton("Play Episode");
    m_podcastPlayButton->setToolTip("Play selected episode");
    m_podcastPlayButton->setObjectName("primaryButton");
    m_podcastPlayButton->setEnabled(false);

    m_podcastStopButton = new QPushButton("Stop");
    m_podcastStopButton->setToolTip("Stop playback");
    m_podcastStopButton->setObjectName("secondaryButton");
    m_podcastStopButton->setEnabled(false);

    buttonLayout->addWidget(m_podcastRefreshButton);
    buttonLayout->addWidget(m_podcastUnsubscribeButton);
    buttonLayout->addWidget(m_podcastPlayButton);
    buttonLayout->addWidget(m_podcastStopButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
    layout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========
    connect(m_podcastRefreshButton, &QPushButton::clicked, this, &MainWindow::onPodcastRefreshClicked);
    connect(m_podcastUnsubscribeButton, &QPushButton::clicked, this, &MainWindow::onPodcastUnsubscribeClicked);
    connect(m_podcastPlayButton, &QPushButton::clicked, [this]() {
        onPodcastPlayEpisodeClicked(m_podcastEpisodeList->currentRow());
    });
    connect(m_podcastStopButton, &QPushButton::clicked, this, &MainWindow::onPodcastStopClicked);
    connect(m_podcastEpisodeList, &QListWidget::itemSelectionChanged, [this]() {
        m_podcastPlayButton->setEnabled(m_podcastEpisodeList->currentRow() >= 0);
    });
    connect(m_podcastEpisodeList, &QListWidget::itemDoubleClicked, [this]() {
        onPodcastPlayEpisodeClicked(m_podcastEpisodeList->currentRow());
    });

    return panel;
}

void MainWindow::updatePodcastGrid()
{
    // Clear existing cards
    QLayoutItem* item;
    while ((item = m_podcastGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_podcastCards.clear();

    if (m_podcastShows.isEmpty()) {
        QLabel* emptyLabel = new QLabel("No podcasts subscribed yet.\n\nUse Tools → Browse Podcasts to add some.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setWordWrap(true);
        emptyLabel->setMinimumHeight(400);
        emptyLabel->setObjectName("emptyMessageLabel");
        m_podcastGrid->addWidget(emptyLabel, 0, 0, 1, 3);
        return;
    }

    int row = 0;
    int col = 0;
    int maxCols = 3;

    for (int i = 0; i < m_podcastShows.size(); ++i) {
        QFrame* card = createPodcastCard(m_podcastShows[i], i);
        m_podcastGrid->addWidget(card, row, col);
        m_podcastCards[i] = card;

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    m_podcastGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, maxCols);
}


void MainWindow::savePodcasts()
{
    QString filePath = JASMINE_CONSTANTS::appDirPath + "/podcasts.dat";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out << quint32(m_podcastShows.size());

        for (const PodcastShow& show : m_podcastShows) {
            out << show.showId;
            out << show.title;
            out << show.author;
            out << show.artworkUrl;
            out << show.feedUrl;
            out << show.websiteUrl;
            out << show.category;
            out << show.description;
            out << show.localArtworkPath;
            out << show.comments;
            out << show.episodeCount;
            out << show.isSubscribed;
            // Save episodes
            out << quint32(show.episodes.size());
            for (const PodcastEpisode& ep : show.episodes) {
                out << ep.guid;
                out << ep.title;
                out << ep.description;
                out << ep.audioUrl;
                out << ep.pubDate;
                out << ep.duration;
                out << ep.playbackPosition;
                out << ep.isPlayed;
                out << ep.isDownloaded;
                out << ep.localFilePath;
            }
        }
        file.close();
    }
}

void MainWindow::loadPodcasts()
{
    QString filePath = JASMINE_CONSTANTS::appDirPath + "/podcasts.dat";
    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);

        quint32 count;
        in >> count;
        m_podcastShows.clear();

        for (quint32 i = 0; i < count; ++i) {
            PodcastShow show;
            in >> show.showId;
            in >> show.title;
            in >> show.author;
            in >> show.artworkUrl;
            in >> show.feedUrl;
            in >> show.websiteUrl;
            in >> show.category;
            in >> show.description;
            in >> show.localArtworkPath;
            in >> show.comments;
            in >> show.episodeCount;
            in >> show.isSubscribed;

            quint32 epCount;
            in >> epCount;
            for (quint32 j = 0; j < epCount; ++j) {
                PodcastEpisode ep;
                in >> ep.guid;
                in >> ep.title;
                in >> ep.description;
                in >> ep.audioUrl;
                in >> ep.pubDate;
                in >> ep.duration;
                in >> ep.playbackPosition;
                in >> ep.isPlayed;
                in >> ep.isDownloaded;
                in >> ep.localFilePath;
                show.episodes.append(ep);
            }
            m_podcastShows.append(show);
        }
        file.close();
    }
}

////// podcast slots

void MainWindow::onPodcastShowSelected(int index)
{
    if (index < 0 || index >= m_podcastShows.size()) return;

    m_currentPodcastIndex = index;
    const PodcastShow& show = m_podcastShows[index];

    // Update artwork
    if (!show.localArtworkPath.isEmpty() && QFile::exists(show.localArtworkPath)) {
        QPixmap pixmap(show.localArtworkPath);
        if (!pixmap.isNull()) {
            m_podcastArtworkLabel->setPixmap(pixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_podcastArtworkLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(120, 120));
        }
    } else if (!show.artworkUrl.isEmpty()) {
        m_podcastArtworkLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(120, 120));
        downloadPodcastArtwork(index);
    } else {
        m_podcastArtworkLabel->setPixmap(QIcon::fromTheme("audio-x-generic").pixmap(120, 120));
    }

    // Update podcast information
    m_podcastTitleLabel->setText(show.title);
    m_podcastAuthorLabel->setText(show.author);
    m_podcastCategoryLabel->setText(show.category);
    m_podcastFeedUrlLabel->setText(show.feedUrl);

    // Website link
    if (!show.websiteUrl.isEmpty()) {
        m_podcastWebsiteLabel->setText(QString("<a href=\"%1\">%1</a>").arg(show.websiteUrl));
    } else {
        m_podcastWebsiteLabel->setText("Not available");
    }

    // Episode count and last updated
    m_podcastEpisodeCountLabel->setText(QString("%1 episodes").arg(show.episodes.size()));
    if (show.lastUpdated.isValid()) {
        m_podcastLastUpdatedLabel->setText(QString("Updated: %1").arg(show.lastUpdated.toString("yyyy-MM-dd")));
    } else {
        m_podcastLastUpdatedLabel->setText("Not updated yet");
    }

    m_podcastDescriptionLabel->setText(show.description);
    m_podcastCommentsEdit->setPlainText(show.comments);

    // Populate episode list
    m_podcastEpisodeList->clear();
    for (const PodcastEpisode& episode : show.episodes) {
        QString durationStr = QString("%1:%2")
            .arg(episode.duration / 60, 2, 10, QChar('0'))
            .arg(episode.duration % 60, 2, 10, QChar('0'));
        QString itemText = QString("%1 - %2 (%3)")
            .arg(episode.pubDate.toString("yyyy-MM-dd"))
            .arg(episode.title)
            .arg(durationStr);
        QListWidgetItem* item = new QListWidgetItem(itemText);
        if (episode.isPlayed) {

        }
        m_podcastEpisodeList->addItem(item);
    }

    // Highlight selected card
    for (int i = 0; i < m_podcastCards.size(); ++i) {
        highlightCard(m_podcastCards[i], i == index);
    }
}

void MainWindow::onPodcastRefreshClicked()
{
    if (m_currentPodcastIndex < 0 || m_currentPodcastIndex >= m_podcastShows.size()) return;

    QString feedUrl = m_podcastShows[m_currentPodcastIndex].feedUrl;
    if (feedUrl.isEmpty()) return;

    // Show loading indicator
    statusBar()->showMessage("Refreshing podcast feed...", 2000);

    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(feedUrl)));

    connect(reply, &QNetworkReply::finished, [this, reply, nam]() {
        if (reply->error() == QNetworkReply::NoError) {
            parseRSSFeed(reply->readAll(), m_currentPodcastIndex);
            savePodcasts();
            updatePodcastGrid();
            onPodcastShowSelected(m_currentPodcastIndex);
            statusBar()->showMessage("Podcast feed refreshed.", 3000);
        } else {
            statusBar()->showMessage("Failed to refresh: " + reply->errorString(), 3000);
        }
        reply->deleteLater();
        nam->deleteLater();
    });
}

void MainWindow::onPodcastUnsubscribeClicked()
{
    if (m_currentPodcastIndex < 0 || m_currentPodcastIndex >= m_podcastShows.size()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Unsubscribe",
        QString("Unsubscribe from \"%1\"?\n\nThis will remove the podcast and all episodes from your collection.")
            .arg(m_podcastShows[m_currentPodcastIndex].title),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // Delete local artwork
        QString artworkPath = m_podcastShows[m_currentPodcastIndex].localArtworkPath;
        if (!artworkPath.isEmpty() && QFile::exists(artworkPath)) {
            QFile::remove(artworkPath);
        }

        m_podcastShows.removeAt(m_currentPodcastIndex);
        savePodcasts();
        updatePodcastGrid();

        if (m_podcastShows.isEmpty()) {
            m_currentPodcastIndex = -1;
            m_podcastArtworkLabel->clear();
            m_podcastTitleLabel->clear();
            m_podcastAuthorLabel->clear();
            m_podcastCategoryLabel->clear();
            m_podcastDescriptionLabel->clear();
            m_podcastEpisodeList->clear();
        } else {
            onPodcastShowSelected(0);
        }

        statusBar()->showMessage("Podcast unsubscribed.", 3000);
    }
}
/*
void MainWindow::onPodcastPlayEpisodeClicked(int episodeIndex)
{
    if (m_currentPodcastIndex < 0 || m_currentPodcastIndex >= m_podcastShows.size()) return;
    if (episodeIndex < 0 || episodeIndex >= m_podcastShows[m_currentPodcastIndex].episodes.size()) return;

    const PodcastEpisode& episode = m_podcastShows[m_currentPodcastIndex].episodes[episodeIndex];
    if (!episode.audioUrl.isEmpty()) {
        player->setUrl(episode.audioUrl);
        player->setWindowTitle(QString("Playing: %1 - %2").arg(
            m_podcastShows[m_currentPodcastIndex].title,
            episode.title));
        player->play();
        player->show();

        statusBar()->showMessage(QString("Now Playing: %1").arg(episode.title), 0);
        m_podcastPlayButton->setEnabled(false);
        m_podcastStopButton->setEnabled(true);
    }
}
*/

void MainWindow::onPodcastPlayEpisodeClicked(int episodeIndex)
{
    if (m_currentPodcastIndex < 0 || m_currentPodcastIndex >= m_podcastShows.size()) return;
    if (episodeIndex < 0 || episodeIndex >= m_podcastShows[m_currentPodcastIndex].episodes.size()) return;

    const PodcastEpisode& episode = m_podcastShows[m_currentPodcastIndex].episodes[episodeIndex];
    if (!episode.audioUrl.isEmpty()) {
        // Clear playing flag from all podcast cards
        for (QFrame* card : m_podcastCards) {
            card->setProperty("playing", false);
            card->style()->polish(card);
        }

        // Set playing flag on current podcast card
        if (m_podcastCards.contains(m_currentPodcastIndex)) {
            m_podcastCards[m_currentPodcastIndex]->setProperty("playing", true);
            m_podcastCards[m_currentPodcastIndex]->style()->polish(m_podcastCards[m_currentPodcastIndex]);
        }

        // Unhighlight all episodes in the list
        for (int i = 0; i < m_podcastEpisodeList->count(); ++i) {
            QListWidgetItem* item = m_podcastEpisodeList->item(i);
            QFont font = item->font();
            font.setBold(false);
            item->setFont(font);
            item->setForeground(QBrush());  // Empty brush = default color
        }

        // Highlight the currently playing episode
        QListWidgetItem* currentItem = m_podcastEpisodeList->item(episodeIndex);
        if (currentItem) {
            QFont font = currentItem->font();
            font.setBold(true);
            currentItem->setFont(font);
            currentItem->setForeground(QColor(170, 102, 255)); // Purple
        }


        showStreamLoadingDialog(7000);
        player->setUrl(episode.audioUrl);
        player->setWindowTitle(QString("Playing: %1 - %2").arg(
            m_podcastShows[m_currentPodcastIndex].title,
            episode.title));
        player->play();
        player->show();

        statusBar()->showMessage(QString("Now Playing: %1").arg(episode.title), 0);
        m_podcastPlayButton->setEnabled(false);
        m_podcastStopButton->setEnabled(true);
    }
}

void MainWindow::onPodcastStopClicked(int episodeindex)
{
    player->stop();
    m_podcastPlayButton->setEnabled(true);
    m_podcastStopButton->setEnabled(false);
    statusBar()->showMessage(QString());

    // Clear playing flag from all podcast cards
    for (QFrame* card : m_podcastCards) {
        card->setProperty("playing", false);
        card->style()->polish(card);
    }

    // Unhighlight all episodes in the list

    for (int i = 0; i < m_podcastEpisodeList->count(); ++i) {
        QListWidgetItem* item = m_podcastEpisodeList->item(i);
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
        item->setForeground(QBrush());  // Empty brush = default color
    }

}

/*
void MainWindow::onPodcastStopClicked()
{
    player->stop();
    m_podcastPlayButton->setEnabled(true);
    m_podcastStopButton->setEnabled(false);
    statusBar()->showMessage(QString());
}
*/


void MainWindow::onAddPodcastFromDialog(const PodcastShow &show)
{
    // Check if already subscribed
    for (const auto& existing : m_podcastShows) {
        if (existing.feedUrl == show.feedUrl) {
            statusBar()->showMessage("Already subscribed to this podcast.", 3000);
            return;
        }
    }

    // Add new show
    PodcastShow newShow = show;
    newShow.isSubscribed = true;
    newShow.episodes.clear();
    newShow.episodeCount = 0;

    m_podcastShows.append(newShow);
    savePodcasts();
    updatePodcastGrid();

    // Fetch episodes from RSS feed
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(newShow.feedUrl)));

    connect(reply, &QNetworkReply::finished, [this, reply, nam, index = m_podcastShows.size() - 1]() {
        if (reply->error() == QNetworkReply::NoError) {
            parseRSSFeed(reply->readAll(), index);
            savePodcasts();
            updatePodcastGrid();
            onPodcastShowSelected(index);
        }
        reply->deleteLater();
        nam->deleteLater();
    });

    // Switch to podcast tab
    if (m_leftPanelTabs->count() > 4) {
        m_leftPanelTabs->setCurrentIndex(4);
    }

    statusBar()->showMessage(QString("Subscribed to: %1").arg(show.title), 3000);
}

void MainWindow::downloadPodcastArtwork(int index)
{
    if (index < 0 || index >= m_podcastShows.size()) return;

    QString artworkUrl = m_podcastShows[index].artworkUrl;
    if (artworkUrl.isEmpty()) return;

    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(artworkUrl)));

    connect(reply, &QNetworkReply::finished, [this, reply, nam, index]() {
        if (reply->error() == QNetworkReply::NoError) {
            QDir().mkpath(JASMINE_CONSTANTS::iconDir);
            QString iconPath = JASMINE_CONSTANTS::iconDir + "/podcast_" +
                              m_podcastShows[index].showId + ".png";

            QFile file(iconPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();

                m_podcastShows[index].localArtworkPath = iconPath;
                savePodcasts();
                updatePodcastGrid();

                if (m_currentPodcastIndex == index) {
                    QPixmap pixmap(iconPath);
                    if (!pixmap.isNull()) {
                        m_podcastArtworkLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                }
            }
        }
        reply->deleteLater();
        nam->deleteLater();
    });
}

void MainWindow::parseRSSFeed(const QByteArray &data, int showIndex)
{
    if (showIndex < 0 || showIndex >= m_podcastShows.size()) return;

    QXmlStreamReader xml(data);
    QVector<PodcastEpisode> episodes;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "item") {
            PodcastEpisode episode;

            while (!(xml.isEndElement() && xml.name() == "item")) {
                xml.readNext();

                if (xml.isStartElement()) {
                    if (xml.name() == "title") {
                        episode.title = xml.readElementText();
                    } else if (xml.name() == "description" || xml.name() == "summary") {
                        episode.description = xml.readElementText();
                    } else if (xml.name() == "pubDate") {
                        episode.pubDate = QDateTime::fromString(xml.readElementText(), Qt::RFC2822Date);
                    } else if (xml.name() == "guid") {
                        episode.guid = xml.readElementText();
                    } else if (xml.name() == "enclosure") {
                        episode.audioUrl = xml.attributes().value("url").toString();
                    } else if (xml.name() == "duration") {
                        episode.duration = xml.readElementText().toInt();
                    }
                }
            }

            if (!episode.audioUrl.isEmpty() && !episode.guid.isEmpty()) {
                episodes.append(episode);
            }
        }
    }

    m_podcastShows[showIndex].episodes = episodes;
    m_podcastShows[showIndex].episodeCount = episodes.size();
    m_podcastShows[showIndex].lastUpdated = QDateTime::currentDateTime();

    if (xml.hasError()) {
        qDebug() << "RSS parse error:" << xml.errorString();
    }
}

void MainWindow::exportPodcastSubscriptions()
{
    QString defaultFileName = JASMINE_CONSTANTS::appDirPath + "/jasmine_podcasts.opml";
    QString filePath = QFileDialog::getSaveFileName(this, "Export Podcast Subscriptions",
                                                    defaultFileName,
                                                    "OPML Files (*.opml);;All Files (*.*)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Cannot write file: " + file.errorString());
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("opml");
    xml.writeAttribute("version", "1.0");

    xml.writeStartElement("head");
    xml.writeTextElement("title", "Jasmine Podcast Subscriptions");
    xml.writeTextElement("dateCreated", QDateTime::currentDateTime().toString(Qt::ISODate));
    xml.writeEndElement(); // head

    xml.writeStartElement("body");

    for (const PodcastShow& show : m_podcastShows) {
        xml.writeStartElement("outline");
        xml.writeAttribute("type", "rss");
        xml.writeAttribute("text", show.title);
        xml.writeAttribute("title", show.title);
        xml.writeAttribute("xmlUrl", show.feedUrl);
        xml.writeAttribute("htmlUrl", show.websiteUrl);
        xml.writeEndElement();
    }

    xml.writeEndElement(); // body
    xml.writeEndElement(); // opml
    xml.writeEndDocument();

    file.close();
    statusBar()->showMessage(QString("Exported %1 subscriptions to %2").arg(m_podcastShows.size()).arg(filePath), 3000);
}

void MainWindow::importPodcastSubscriptions()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Import Podcast Subscriptions",
                                                    JASMINE_CONSTANTS::appDirPath,
                                                    "OPML Files (*.opml);;All Files (*.*)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot read file: " + file.errorString());
        return;
    }

    QXmlStreamReader xml(&file);
    int importedCount = 0;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "outline") {
            QString type = xml.attributes().value("type").toString();
            if (type == "rss" || type.isEmpty()) {
                QString feedUrl = xml.attributes().value("xmlUrl").toString();
                QString title = xml.attributes().value("title").toString();
                if (title.isEmpty()) {
                    title = xml.attributes().value("text").toString();
                }
                QString websiteUrl = xml.attributes().value("htmlUrl").toString();

                if (!feedUrl.isEmpty()) {
                    // Check if already subscribed
                    bool exists = false;
                    for (const auto& existing : m_podcastShows) {
                        if (existing.feedUrl == feedUrl) {
                            exists = true;
                            break;
                        }
                    }

                    if (!exists) {
                        PodcastShow newShow;
                        newShow.feedUrl = feedUrl;
                        newShow.title = title;
                        newShow.websiteUrl = websiteUrl;
                        newShow.isSubscribed = true;
                        newShow.showId = QUuid::createUuid().toString();

                        m_podcastShows.append(newShow);
                        importedCount++;

                    // Fetch the RSS feed to get artwork, description, etc.
                       QNetworkAccessManager* nam = new QNetworkAccessManager(this);
                       QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(feedUrl)));
                       int newIndex = m_podcastShows.size() - 1;

                       connect(reply, &QNetworkReply::finished, [this, reply, nam, newIndex]() {
                           if (reply->error() == QNetworkReply::NoError) {
                               // This will update episodes, artwork, description
                               parseRSSFeed(reply->readAll(), newIndex);
                               savePodcasts();
                               updatePodcastGrid();
                               if (m_currentPodcastIndex == newIndex) {
                                   onPodcastShowSelected(newIndex);
                               }
                           }
                           reply->deleteLater();
                           nam->deleteLater();
                       });
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        QMessageBox::warning(this, "Error", "Failed to parse OPML: " + xml.errorString());
    }

    file.close();

    if (importedCount > 0) {
        savePodcasts();
        updatePodcastGrid();
        statusBar()->showMessage(QString("Imported %1 podcasts from %2").arg(importedCount).arg(filePath), 3000);
    } else {
        statusBar()->showMessage("No new podcasts found in OPML file.", 3000);
    }
}

void MainWindow::refreshAllPodcasts()
{
    if (m_podcastShows.isEmpty()) {
        statusBar()->showMessage("No podcasts to refresh.", 2000);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Refresh All Podcasts",
        QString("Refresh all %1 podcasts?\n\nThis will check for new episodes in each feed.").arg(m_podcastShows.size()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    statusBar()->showMessage("Refreshing all podcasts...", 2000);

    int total = m_podcastShows.size();
    int completed = 0;

    for (int i = 0; i < total; ++i) {
        QString feedUrl = m_podcastShows[i].feedUrl;
        if (feedUrl.isEmpty()) continue;

        QNetworkAccessManager* nam = new QNetworkAccessManager(this);
        QNetworkReply* reply = nam->get(QNetworkRequest(QUrl(feedUrl)));

        connect(reply, &QNetworkReply::finished, [this, reply, nam, i, total, &completed]() {
            if (reply->error() == QNetworkReply::NoError) {
                parseRSSFeed(reply->readAll(), i);
            }
            reply->deleteLater();
            nam->deleteLater();

            completed++;
            if (completed == total) {
                savePodcasts();
                updatePodcastGrid();
                if (m_currentPodcastIndex >= 0) {
                    onPodcastShowSelected(m_currentPodcastIndex);
                }
                statusBar()->showMessage("All podcasts refreshed.", 3000);
            }
        });
    }
}
