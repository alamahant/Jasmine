#include "urlbar.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>

URLBar::URLBar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

void URLBar::setupUI()
{
    setFixedHeight(40);
    setObjectName("urlBar");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    // Create navigation buttons
    m_backButton = new QPushButton();
    m_backButton->setIcon(QIcon(":/resources/icons/arrow-left.svg"));
    m_backButton->setFixedSize(32, 32);
    m_backButton->setToolTip("Back");
    m_backButton->setObjectName("urlBarButton");

    m_forwardButton = new QPushButton();
    m_forwardButton->setIcon(QIcon(":/resources/icons/arrow-right.svg"));
    m_forwardButton->setFixedSize(32, 32);
    m_forwardButton->setToolTip("Forward");
    m_forwardButton->setObjectName("urlBarButton");

    m_reloadButton = new QPushButton();
    m_reloadButton->setIcon(QIcon(":/resources/icons/refresh-cw.svg"));
    m_reloadButton->setFixedSize(32, 32);
    m_reloadButton->setToolTip("Reload");
    m_reloadButton->setObjectName("urlBarButton");

    m_homeButton = new QPushButton();
    m_homeButton->setIcon(QIcon(":/resources/icons/home.svg"));
    m_homeButton->setFixedSize(32, 32);
    m_homeButton->setToolTip("Home");
    m_homeButton->setObjectName("urlBarButton");

    // Create URL input
    m_urlInput = new QLineEdit();
    m_urlInput->setObjectName("formInput");
    m_urlInput->setPlaceholderText("Enter URL...");
    m_urlInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // make it readonly
    //m_urlInput->setReadOnly(true);
    // Create utility buttons


    m_copyButton = new QPushButton();
    m_copyButton->setVisible(false);
    m_copyButton->setIcon(QIcon(":/resources/icons/copy.svg"));
    m_copyButton->setFixedSize(32, 32);
    m_copyButton->setToolTip("Copy URL");
    m_copyButton->setObjectName("urlBarButton");

    m_addWebsiteButton = new QPushButton();
    m_addWebsiteButton->setVisible(false);
    m_addWebsiteButton->setIcon(QIcon(":/resources/icons/file-plus.svg"));
    m_addWebsiteButton->setFixedSize(32, 32);
    m_addWebsiteButton->setToolTip("Add to Websites");
    m_addWebsiteButton->setObjectName("urlBarButton");


    // Create search engine selector
    m_searchEngineCombo = new QComboBox();
    m_searchEngineCombo->addItem(QIcon(":/resources/icons/search.svg"), "Google");
    m_searchEngineCombo->addItem(QIcon(":/resources/icons/search.png"), "Duck2Go");
    m_searchEngineCombo->addItem(QIcon(":/resources/icons/search.svg"), "Bing");
    m_searchEngineCombo->setFixedWidth(120);
    m_searchEngineCombo->setToolTip("Select Search Engine");
    m_searchEngineCombo->setObjectName("searchEngineCombo");

    m_openTabButton = new QPushButton();
    m_openTabButton->setIcon(QIcon(":/resources/icons/plus.svg"));
    m_openTabButton->setFixedSize(32, 32);
    m_openTabButton->setToolTip("New Tab");
    m_openTabButton->setObjectName("openTabButton");

    // Add widgets to layout
    layout->addWidget(m_backButton);
    layout->addWidget(m_forwardButton);
    layout->addWidget(m_reloadButton);
    layout->addWidget(m_homeButton);

    layout->addWidget(m_urlInput);
    layout->addWidget(m_searchEngineCombo);  // Add here

    layout->addWidget(m_copyButton);
    layout->addWidget(m_addWebsiteButton);
    layout->addWidget(m_openTabButton);


    // Initially disable navigation buttons
    updateButtonStates();
}

void URLBar::setupConnections()
{
    connect(m_urlInput, &QLineEdit::returnPressed, this, &URLBar::onUrlEditingFinished);
    connect(m_backButton, &QPushButton::clicked, this, &URLBar::onBackClicked);
    connect(m_forwardButton, &QPushButton::clicked, this, &URLBar::onForwardClicked);
    connect(m_reloadButton, &QPushButton::clicked, this, &URLBar::onReloadClicked);
    connect(m_homeButton, &QPushButton::clicked, this, &URLBar::onHomeClicked);
    connect(m_copyButton, &QPushButton::clicked, this, &URLBar::onCopyClicked);
    connect(m_addWebsiteButton, &QPushButton::clicked, this, &URLBar::onAddWebsiteClicked);
    connect(m_openTabButton, &QPushButton::clicked, this, &URLBar::onOpenNewTab);
    connect(m_searchEngineCombo, &QComboBox::currentTextChanged, this, &URLBar::onSearchEngineChanged);


}

void URLBar::setUrl(const QString &url)
{
    if (m_urlInput->text() != url) {
        m_urlInput->setText(url);
    }
}

void URLBar::updateNavigationState(bool canGoBack, bool canGoForward)
{
    m_backButton->setEnabled(canGoBack);
    m_forwardButton->setEnabled(canGoForward);
}



void URLBar::onUrlEditingFinished()
{
    QString url = m_urlInput->text().trimmed();
    if (!url.isEmpty()) {
        // Add protocol if missing
        if (!url.startsWith("http://") && !url.startsWith("https://") && !url.startsWith("file://")) {
            url = "https://" + url;
        }
        emit urlChanged(url);
    }
}

void URLBar::onBackClicked()
{
    emit backRequested();
}

void URLBar::onForwardClicked()
{
    emit forwardRequested();
}

void URLBar::onReloadClicked()
{
    emit reloadRequested();
}

void URLBar::onHomeClicked()
{
    emit homeRequested();
}

void URLBar::onCopyClicked()
{
    QString url = m_urlInput->text();
    if (!url.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(url);
        emit copyUrlRequested();
    }
}

void URLBar::updateButtonStates()
{
    // Initially disable navigation buttons until we have a web view
    //m_backButton->setEnabled(false);
    //m_forwardButton->setEnabled(false);
}

void URLBar::updateTheme(bool isDarkTheme) {
    QString iconPath = isDarkTheme ? ":/resources/icons-white/" : ":/resources/icons/";

    m_backButton->setIcon(QIcon(iconPath + "arrow-left.svg"));
    m_forwardButton->setIcon(QIcon(iconPath + "arrow-right.svg"));
    m_reloadButton->setIcon(QIcon(iconPath + "refresh-cw.svg"));
    m_homeButton->setIcon(QIcon(iconPath + "home.svg"));
    m_copyButton->setIcon(QIcon(iconPath + "copy.svg"));
    m_addWebsiteButton->setIcon(QIcon(iconPath + "file-plus.svg"));
    m_openTabButton->setIcon(QIcon(iconPath + "plus.svg"));
    // Update search engine combo icons
    if (m_searchEngineCombo) {
        QIcon searchIcon(iconPath + "search.svg");
        for (int i = 0; i < m_searchEngineCombo->count(); ++i) {
            m_searchEngineCombo->setItemIcon(i, searchIcon);
        }
    }

}

void URLBar::onAddWebsiteClicked(){
    emit addWebsiteRequested();
}

void URLBar::onOpenNewTab()
{
    emit newTabRequested();

}

void URLBar::onSearchEngineChanged(const QString &text) {
    // Do something when search engine changes
    // Maybe emit a signal or update UI
    emit searchEngineChanged(text);
}

QUrl URLBar::getSearchUrl() const {
    QString currentEngine = m_searchEngineCombo->currentText();

    if (currentEngine == "Google") {
        return QUrl("https://www.google.com/search?q=");
    } else if (currentEngine == "Duck2Go") {
        return QUrl("https://duckduckgo.com/?q=");
    } else if (currentEngine == "Bing") {
        return QUrl("https://www.bing.com/search?q=");
    }

    // Default to Google
    return QUrl("https://www.google.com/search?q=");
}

