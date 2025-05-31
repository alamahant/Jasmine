
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
    setWindowTitle("Jasmine");
    setWindowIcon(QIcon(":/resources/jasmine.png"));
    resize(1100, 800);

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
    new QShortcut(QKeySequence("Ctrl+L"), this, SLOT(onLaunchWebsite()));
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

    statusBar()->showMessage("Ready");
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

}

QWidget* MainWindow::createModernDashboard() {
    QWidget* dashboard = new QWidget(this);

    // Main layout
    QVBoxLayout* containerLayout = new QVBoxLayout(dashboard);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);


    // Create main horizontal splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(1);
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
    QStackedWidget* detailsStack = new QStackedWidget();

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
    mainSplitter->setSizes(QList<int>() << 300 << 400);

    connect(m_leftPanelTabs, &QTabWidget::currentChanged, [this, detailsStack](int index) {
        // Switch the details stack to match the tab
        detailsStack->setCurrentIndex(index);

        // Restore selection based on the active tab
        if (index == 0) { // Websites tab
            if (m_currentWebsiteIndex >= 0 && m_currentWebsiteIndex < m_model->rowCount()) {
                // Reselect the previously selected website
                QModelIndex modelIndex = m_model->index(m_currentWebsiteIndex, 0);
                onWebsiteSelected(modelIndex);
            } else if (m_model->rowCount() > 0) {
                // Select the first website if none was previously selected
                QModelIndex firstIndex = m_model->index(0, 0);
                onWebsiteSelected(firstIndex);
            }
        } else if (index == 1) { // Sessions tab
            if (!m_currentSessionName.isEmpty() && m_sessions.contains(m_currentSessionName)) {
                // Reselect the previously selected session
                onSessionSelected(m_currentSessionName);
            } else if (!m_sessions.isEmpty()) {
                // Select the first session if none was previously selected
                QString firstName = m_sessions.keys().first();
                onSessionSelected(firstName);
            }
        }
    });

    // Create a hidden list view for backward compatibility
    m_websiteList = new QListView();
    m_websiteList->setVisible(false);

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
    m_addButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_editButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_launchButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));

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

    // Create tab widget
    m_tabWidget = new QTabWidget();
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
    card->setFixedSize(180, 180); // Square cards
    card->setCursor(Qt::ArrowCursor);  // Always arrow, never hand


    // Add shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);

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
    editBtn->setToolTip("Edit Website");
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
        m_websiteList->setCurrentIndex(m_model->index(index, 0));
        onWebsiteSelected(m_model->index(index, 0));
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
    card->setFixedSize(180, 180); // Square cards
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
    layout->setContentsMargins(12, 12, 12, 12);

    // Title row with icon and name
    QHBoxLayout* titleLayout = new QHBoxLayout();

    // Icon display
    QLabel* iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(24, 24);
    iconLabel->setScaledContents(true);
    if (!session.icon.isNull()) {
        QPixmap pixmap = session.icon.pixmap(24, 24);
        iconLabel->setPixmap(pixmap);
    }
    titleLayout->addWidget(iconLabel);

    // Session name
    QLabel* nameLabel = new QLabel(sessionName);
    nameLabel->setObjectName("cardTitle");
    nameLabel->setWordWrap(true);
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
        if (QMessageBox::question(this, "Delete Session",
                                  QString("Are you sure you want to delete the session '%1'?").arg(sessionName),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            onSessionSelected(sessionName);
            deleteSession(sessionName);
            updateSessionCards();
        }
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
    QWebEngineView *webView = new QWebEngineView(m_tabWidget);

    // Determine which profile to use
    QWebEngineProfile *profile;
    if (m_usingSeparateProfiles) {
        profile = createProfileForTab();
        m_tabProfiles[webView] = profile;
    } else {
        profile = m_webProfile;
    }

    // Create page with the selected profile
    QWebEnginePage *page = new QWebEnginePage(profile, webView);
    webView->setPage(page);
    //

    // Connect downloads to download manager

    // To this:
    if (m_usingSeparateProfiles) {
        connect(profile, &QWebEngineProfile::downloadRequested,
                downloadManager, &DownloadManager::handleDownloadRequest);
    }


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
        m_toggleViewAction->setText("To WebView");
        setWindowTitle("Jasmine");
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



void MainWindow::showWebViews() {
    // Switch to web view
    m_stackedWidget->setCurrentWidget(m_webViewContainer);
    m_stackedWidget->update();
    m_toggleViewAction->setText("To Dashboard");
    setWindowTitle("Jasmine - Web Browser");

    // Always keep the toggle button enabled
    m_toggleViewAction->setEnabled(true);

    // Update tab count
    updateTabCountStatus();
}

void MainWindow::showDashboard() {
    // Switch to dashboard
    m_stackedWidget->setCurrentWidget(m_dashboardWidget);
    m_stackedWidget->update();
    m_toggleViewAction->setText("To WebView");
    setWindowTitle("Jasmine");
    //
    //selectFirstItemIfNoneSelected();
    //
    // Always keep the toggle button enabled

    m_toggleViewAction->setEnabled(true);

    // Update tab count
    updateTabCountStatus();

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
    QMenu* helpMenu = menuBar()->addMenu("Help");
    //security menu
    m_securityManager->setupSecurityMenu(menuBar());

    // Add existing toolbar actions to menus
    fileMenu->addAction(m_addCurrentSessionAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_screenshotAction);
    fileMenu->addAction(m_openDownloadsFolderAction);
    fileMenu->addSeparator();

    editMenu->addAction(m_copyUrlAction);
    editMenu->addSeparator();
    editMenu->addAction(m_addCurrentWebsiteAction);

    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_toggleViewAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_downloadsAction);
    viewMenu->addAction(m_closeAllTabsAction);

    navigateMenu->addAction(m_webBackAction);
    navigateMenu->addAction(m_webForwardAction);
    navigateMenu->addAction(m_webReloadAction);

    // Create new actions for sessions menu
    QAction* saveSessionAction = sessionMenu->addAction("Save Current Session");
    QAction* loadSessionAction = sessionMenu->addAction("Load Session");
    QAction* manageSessionsAction = sessionMenu->addAction("Manage Sessions");
    sessionMenu->addSeparator();
    QAction* cleanAllDataAction = sessionMenu->addAction("Clean Current Session Data");
    QAction* cleanSharedDataAction = sessionMenu->addAction("Clean Shared Profile Data");
    //QAction* factoryResetAction = sessionMenu->addAction("Restore Factory Defaults", this);
    QAction* factoryResetAction = new QAction("Restore Factory Defaults", this);
    sessionMenu->addAction(factoryResetAction);
    // Create new actions for tools menu
    QAction* separateProfilesAction = toolsMenu->addAction("Toggle Private Profile");
    QAction* themeAction = toolsMenu->addAction("Toggle Theme");

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
        dialog.exec();
    });

    connect(featuresAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Features, this);
        dialog.exec();
    });

    connect(instructionsAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Instructions, this);
        dialog.exec();
    });
    connect(securityAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Security, this);
        dialog.exec();
    });
    connect(twofaAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::TwoFA, this);
        dialog.exec();
    });

    connect(dataManagementAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::DataManagement, this);
        dialog.exec();
    });
    connect(downloadManagementAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::DownloadManagement, this);
        dialog.exec();
    });
    connect(onSitesandSessiosAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onSitesAndSessions, this);
        dialog.exec();
    });
    connect(onSecurityAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onSecurity, this);
        dialog.exec();
    });
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

    // Remove this line: if (m_sessions.isEmpty()) { return; }

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

    // Set the profile mode to what it was when session was saved
    m_usingSeparateProfiles = session.usingSeparateProfiles;

    // Restore tabs based on their actual profile state
    for (int i = 0; i < session.openTabUrls.size(); i++) {
        QWebEngineView* webView = new QWebEngineView(m_tabWidget);
        QWebEngineProfile* profile;

        // Check if THIS specific tab had a separate profile when saved
        bool tabHadSeparateProfile = (i < session.tabHasSeparateProfile.size()) ?
                                         session.tabHasSeparateProfile[i] : false;

        if (tabHadSeparateProfile) {
            // This tab had its own profile - restore it
            QString sessionProfileName = "Session_" + name + "_Tab_" + QString::number(i);
            QString sessionProfilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                         "/profiles/" + sessionProfileName;

            profile = new QWebEngineProfile(sessionProfileName, this);
            profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
            profile->setPersistentStoragePath(sessionProfilePath);
            profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
            profile->setCachePath(sessionProfilePath + "/cache");

            m_tabProfiles[webView] = profile;

            // Connect cleanup for separate profile
            connect(webView, &QObject::destroyed, this, [this, webView]() {
                cleanupTabProfile(webView);
            });
        } else {
            // This tab used the shared profile
            profile = m_webProfile;
        }

        QWebEnginePage* page = new QWebEnginePage(profile, webView);
        webView->setPage(page);

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
            int ret = QMessageBox::question(this, "Session Exists",
                                            QString("Session '%1' already exists. Overwrite?").arg(name),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) {
                return; // User chose not to overwrite
            }
        }

        saveSession(name);
        updateSessionCards();
    }
}

void MainWindow::saveSession(const QString& name) {
    // Don't save if name is empty
    if (name.isEmpty()) {
        return;
    }

    SessionData session;
    session.timestamp = QDateTime::currentDateTime();
    session.name = name;
    session.usingSeparateProfiles = m_usingSeparateProfiles;  // Current toggle state for reference

    // First, force all web views to save their state by executing a JavaScript function
    for (int i = 0; i < m_tabWidget->count(); i++) {
        QWebEngineView* view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        /*
        if (view) {
            // Execute a simple script to ensure the page has committed any pending state
            view->page()->runJavaScript("localStorage['_saveMarker'] = new Date().toString();");
        }
        */
        if (view && m_tabProfiles.contains(view)) {
            QWebEngineProfile* profile = m_tabProfiles[view];

            // Force profile to write pending data
            profile->clearHttpCache(); // This forces a sync

            // Or try this alternative:
            // profile->cookieStore()->deleteAllCookies();
            // profile->cookieStore()->loadAllCookies();
        }
    }

    // Wait longer to ensure data is written
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    // Now store open tabs and copy profile data based on actual tab state
    for (int i = 0; i < m_tabWidget->count(); i++) {
        QWebEngineView* view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(i));
        if (view) {
            session.openTabUrls.append(view->url().toString());
            session.openTabTitles.append(m_tabWidget->tabText(i));

            // Store the favicon
            QIcon icon = m_tabWidget->tabIcon(i);
            session.openTabIcons.append(icon);

            // Check if THIS specific tab actually has a separate profile
            bool tabHasSeparateProfile = m_tabProfiles.contains(view);
            session.tabHasSeparateProfile.append(tabHasSeparateProfile);

            if (tabHasSeparateProfile) {
                // This tab has its own profile
                QWebEngineProfile* currentProfile = m_tabProfiles[view];
                //session.tabOriginalProfileNames.append(currentProfile->profileName());

                // Get the current profile's storage path
                QString currentPath = currentProfile->persistentStoragePath();

                // Create a session-specific profile name and path
                QString sessionProfileName = "Session_" + name + "_Tab_" + QString::number(i);
                QString sessionProfilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                             "/profiles/" + sessionProfileName;

                // Delete the destination directory if it already exists
                QDir destDir(sessionProfilePath);
                if (destDir.exists()) {
                    destDir.removeRecursively();
                }

                // Copy the profile data
                if (copyProfileData(currentPath, sessionProfilePath)) {
                } else {
                    qWarning() << "Failed to copy profile data for tab" << i;
                }
            } else {
                // This tab uses the shared profile
                session.tabOriginalProfileNames.append("");  // Empty string indicates shared profile
            }
        }
    }

    // Generate session icon
    session.icon = generateRandomSvgIcon();
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/session_icons/";
    if (!QDir(iconDir).exists()) {
        QDir().mkpath(iconDir);
    }
    QString iconPath = iconDir + name + "_session.png";
    QPixmap pixmap = session.icon.pixmap(64, 64);
    pixmap.save(iconPath, "PNG");

    // Save to sessions map
    m_sessions[name] = session;
    m_currentSessionName = name;

    // Save to disk
    saveSessionsData();

    // Update UI
    updateSessionCards();

    // Update status
    statusBar()->showMessage("Session saved: " + name, 3000);
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
    if (QMessageBox::question(this, "Clean Current Session Data",
                              "Are you sure you want to clean browsing data from current sessions? This will remove cookies, cache, and browsing data from active sessions and shared profile.",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // Clear main profile data
        m_webProfile->clearAllVisitedLinks();
        m_webProfile->clearHttpCache();
        m_webProfile->cookieStore()->deleteAllCookies();

        // Clear tab profiles
        for (QWebEngineProfile* profile : m_tabProfiles.values()) {
            profile->clearAllVisitedLinks();
            profile->clearHttpCache();
            profile->cookieStore()->deleteAllCookies();
        }

        // Show a status message
        statusBar()->showMessage("Current session data has been cleared.", 3000);
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
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->setPersistentStoragePath(profilePath);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setCachePath(profilePath + "/cache");


    return profile;
}

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

        // Get all subdirectories
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        // Find and remove directories matching the session pattern
        QString sessionPrefix = "Session_" + sessionName + "_Tab_";
        for (const QString& entry : entries) {
            if (entry.startsWith(sessionPrefix)) {
                QString fullPath = dir.absoluteFilePath(entry);

                QDir sessionDir(fullPath);
                if (sessionDir.removeRecursively()) {
                } else {
                    qWarning() << "  Failed to remove directory";
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
        m_tabWidget->removeTab(0);
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
    m_sessionUpdateButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_sessionDeleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_sessionLoadButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_sessionClearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));

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

        // Check if this tab has a separate profile
        bool hasSeparateProfile = (i < session.tabHasSeparateProfile.size()) ?
                                      session.tabHasSeparateProfile[i] : false;

        QString profileInfo = hasSeparateProfile ? " [Private Profile]" : " [Shared Profile]";

        // Create item with title, URL, and profile info
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(title + profileInfo + "\n" + url);
        item->setToolTip(url + "\nProfile: " + (hasSeparateProfile ? "Private" : "Shared"));

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
    QToolBar *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    // Web navigation actions - always visible
    m_webBackAction = toolbar->addAction(QIcon(":/resources/icons/arrow-left.svg"), "Back");
    m_webForwardAction = toolbar->addAction(QIcon(":/resources/icons/arrow-right.svg"), "Forward");
    m_webReloadAction = toolbar->addAction(QIcon(":/resources/icons/refresh-cw.svg"), "Reload");

    // Add separator
    toolbar->addSeparator();

    // Add session and website actions - always visible
    m_addCurrentSessionAction = toolbar->addAction(QIcon(":/resources/icons/save.svg"), "Save Current Session");
    m_addCurrentWebsiteAction = toolbar->addAction(QIcon(":/resources/icons/plus.svg"), "Add Currently Selected Website. Open multiple tabs of same site");

    // Add separator
    toolbar->addSeparator();

    // Add zoom actions - always visible
    m_zoomInAction = toolbar->addAction(QIcon(":/resources/icons/zoom-in.svg"), "Zoom In");
    m_zoomOutAction = toolbar->addAction(QIcon(":/resources/icons/zoom-out.svg"), "Zoom Out");

    // Add separator
    toolbar->addSeparator();

    // Add copy URL action - always visible
    m_copyUrlAction = toolbar->addAction(QIcon(":/resources/icons/copy.svg"), "Copy Tab URL");

    // Add separator
    toolbar->addSeparator();

    m_toggleViewAction = new QAction(QIcon(":/resources/icons/monitor.svg"), "Dashboard");
    m_toggleViewAction->setToolTip("Switch between Dashboard and Web View");

    // Create a custom tool button that will display both icon and text
    QToolButton* toggleButton = new QToolButton();
    toggleButton->setDefaultAction(m_toggleViewAction);
    toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Add the button to the toolbar
    toolbar->addWidget(toggleButton);

    // Add tab count label
    m_tabCountLabel = new QLabel("Tabs: 0", this);
    m_tabCountLabel->setAlignment(Qt::AlignCenter);
    m_tabCountLabel->setMinimumWidth(80);
    toolbar->addWidget(m_tabCountLabel);

    toolbar->addSeparator();

    m_downloadsAction = toolbar->addAction(QIcon(":/resources/icons/download.svg"), "Downloads");
    m_downloadsAction->setToolTip("Show Downloads");
    m_originalDownloadIcon = m_downloadsAction->icon(); // Store current icon

    //make the button green when in progress

    //
    toolbar->addSeparator();

    //close tabs
    // Add close all tabs button - always visible
    m_closeAllTabsAction = toolbar->addAction(QIcon(":/resources/icons/x.svg"), "Close All Tabs");
    m_closeAllTabsAction->setToolTip("Close All Open Tabs");

    //seperateprofiles toggle
    toolbar->addSeparator();
    QLabel *profileLabel = new QLabel("Private Profile:");
    toolbar->addWidget(profileLabel);

    // Create modern toggle switch
    m_separateProfilesToggle = new QPushButton();
    m_separateProfilesToggle->setCheckable(true);
    m_separateProfilesToggle->setFixedSize(50, 25);
    /*
    m_separateProfilesToggle->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #ccc;"
        "    border-radius: 12px;"
        "    background-color: #f0f0f0;"
        "    text-align: center;"
        "}"
        "QPushButton:checked {"
        "    background-color: #4CAF50;"
        "    border-color: #4CAF50;"
        "}"
        "QPushButton::indicator {"
        "    width: 20px;"
        "    height: 20px;"
        "    border-radius: 10px;"
        "    background-color: white;"
        "    border: 1px solid #ccc;"
        "}"
        );
    */

    m_separateProfilesToggle->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #ccc;"
        "    border-radius: 12px;"
        //"    background-color: #8e8e93;"
        "    background-color: #808080;"        // Pure gray (equal RGB values)
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
    toolbar->addWidget(m_separateProfilesToggle);

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

    //screenshot
    toolbar->addSeparator();
    m_screenshotAction = toolbar->addAction(QIcon(":/resources/icons/camera.svg"), "Screenshot");
    m_screenshotAction->setToolTip("Take Screenshot of Current Tab");

    // Connect screenshot action
    connect(m_screenshotAction, &QAction::triggered, this, &MainWindow::takeScreenshot);

    //open downloads location
    toolbar->addSeparator();
    m_openDownloadsFolderAction = toolbar->addAction(QIcon(":/resources/icons/folder.svg"), "Open Downloads Folder");
    m_openDownloadsFolderAction->setToolTip("Open Downloads Folder");
    //2FA
    toolbar->addSeparator();
    m_open2faManagerAction = toolbar->addAction(QIcon(":/resources/icons/shield.svg"), "2FAManager");
    m_open2faManagerAction->setToolTip("Open 2FA Manager");
    connect(m_open2faManagerAction, &QAction::triggered, this, &MainWindow::on_Open2faManager);
    //
    connect(m_openDownloadsFolderAction, &QAction::triggered, this, &MainWindow::openDownloadsFolder);

    //
    // Connect separate profiles toggle
    connect(m_separateProfilesToggle, &QPushButton::toggled, [this](bool checked) {
        m_usingSeparateProfiles = checked;
        m_separateProfilesToggle->setText(checked ? "ON" : "OFF");
        m_separateProfilesToggle->setToolTip(checked ? "Using private profile" : "Using shared profile");
    });

    //
    // Connect close all tabs action
    connect(m_closeAllTabsAction, &QAction::triggered, [this]() {
        while (m_tabWidget->count() > 0) {
            onTabCloseRequested(0);
        }
    });

    //
    // Connect downloads action
    connect(m_downloadsAction, &QAction::triggered, [this]() {
        downloadManager->showDownloadsWindow();
    });

    //
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

    return toolbar;
}

void MainWindow::toggleView() {
    if (m_stackedWidget->currentWidget() == m_dashboardWidget) {
        // Currently in dashboard, switch to web view
        m_stackedWidget->setCurrentWidget(m_webViewContainer);
        m_toggleViewAction->setText("To Dashboard");
        setWindowTitle("Jasmine - Web Browser");

        // Force update
        m_stackedWidget->update();
    } else {
        // Currently in web view, switch to dashboard
        m_stackedWidget->setCurrentWidget(m_dashboardWidget);
        m_toggleViewAction->setText("To WebView");
        setWindowTitle("Jasmine");

        // Force update
        m_stackedWidget->update();
    }

    // Always keep the toggle button enabled
    m_toggleViewAction->setEnabled(true);

    // Update tab count
    updateTabCountStatus();
}

void MainWindow::onAddCurrentWebsite()
{

}

void MainWindow::updateTabCountStatus() {
    int tabCount = m_tabWidget->count();
    // Update the tab count label
    m_tabCountLabel->setText(QString("Tabs: %1").arg(tabCount));
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


/*
void MainWindow::takeScreenshot()
{
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

    QString downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Jasmine";


    QDir downloadsDir(downloadDirectory);
    if (!downloadsDir.exists()) {
        downloadsDir.mkpath(".");
    }

    QString filepath = downloadsDir.filePath(filename);

    if (screenshot.save(filepath)) {
        QMessageBox::information(this, "Screenshot", "Screenshot saved: " + filename);
    } else {
        QMessageBox::warning(this, "Error", "Failed to save screenshot.");
    }
}
*/

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
#ifdef FLATPAK_BUILD
    QString downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) = "/Downloads";
#else
    QString downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Jasmine";
#endif
    QDir downloadsDir(downloadDirectory);
    if (!downloadsDir.exists()) {
        downloadsDir.mkpath(".");
    }
    QString filepath = downloadsDir.filePath(filename);
    if (screenshot.save(filepath)) {
        QMessageBox::information(this, "Screenshot", "Screenshot saved: " + filename);
    } else {
        QMessageBox::warning(this, "Error", "Failed to save screenshot.");
    }
}



void MainWindow::openDownloadsFolder(){

#ifdef FLATPAK_BUILD
    QString downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Downloads";
#else
    QString downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Jasmine";
#endif

    QDir downloadsDir(downloadDirectory);

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
                       .arg(downloadDirectory));
    msgBox.setStandardButtons(QMessageBox::Ok);
    QPushButton *copyButton = msgBox.addButton("Copy Path", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QApplication::clipboard()->setText(downloadDirectory);
    }
#else
    // Open in system file manager
    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadDirectory));
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
        m_toggleViewAction->setIcon(QIcon(":/resources/icons-white/monitor.svg"));
        m_downloadsAction->setIcon(QIcon(":/resources/icons-white/download.svg"));
        m_closeAllTabsAction->setIcon(QIcon(":/resources/icons-white/x.svg"));
        m_screenshotAction->setIcon(QIcon(":/resources/icons-white/camera.svg"));
        m_openDownloadsFolderAction->setIcon(QIcon(":/resources/icons-white/folder.svg"));
        m_leftPanelTabs->setTabIcon(0, createRotatedIcon(":/resources/icons-white/globe.svg"));
        m_leftPanelTabs->setTabIcon(1, createRotatedIcon(":/resources/icons-white/bookmark.svg"));
        m_open2faManagerAction->setIcon(QIcon(":/resources/icons-white/shield.svg"));
        m_usernameEyeButton->setIcon(QIcon(":/resources/icons-white/eye.svg"));
        m_passwordEyeButton->setIcon(QIcon(":/resources/icons-white/eye.svg"));

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
        m_toggleViewAction->setIcon(QIcon(":/resources/icons/monitor.svg"));
        m_downloadsAction->setIcon(QIcon(":/resources/icons/download.svg"));
        m_closeAllTabsAction->setIcon(QIcon(":/resources/icons/x.svg"));
        m_screenshotAction->setIcon(QIcon(":/resources/icons/camera.svg"));
        m_openDownloadsFolderAction->setIcon(QIcon(":/resources/icons/folder.svg"));
        m_leftPanelTabs->setTabIcon(0, createRotatedIcon(":/resources/icons/globe.svg"));
        m_leftPanelTabs->setTabIcon(1, createRotatedIcon(":/resources/icons/bookmark.svg"));
        m_open2faManagerAction->setIcon(QIcon(":/resources/icons/shield.svg"));
        m_usernameEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));
        m_passwordEyeButton->setIcon(QIcon(":/resources/icons/eye.svg"));

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





