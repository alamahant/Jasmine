#ifndef URLBAR_H
#define URLBAR_H

#include <QWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QComboBox>

class URLBar : public QWidget
{
    Q_OBJECT

public:
    explicit URLBar(QWidget *parent = nullptr);
    void setUrl(const QString &url);
    void updateNavigationState(bool canGoBack, bool canGoForward);
    void updateTheme(bool isDarkTheme);
    QUrl getSearchUrl() const;

signals:
    void urlChanged(const QString &url);
    void backRequested();
    void forwardRequested();
    void reloadRequested();
    void homeRequested();
    void copyUrlRequested();
    void addWebsiteRequested();
    void newTabRequested();
    void searchEngineChanged(const QString& text);

private slots:
    void onUrlEditingFinished();
    void onBackClicked();
    void onForwardClicked();
    void onReloadClicked();
    void onHomeClicked();
    void onCopyClicked();
    void onAddWebsiteClicked();
    void onOpenNewTab();
    void onSearchEngineChanged(const QString &text);

private:
    QLineEdit *m_urlInput;
    QPushButton *m_backButton;
    QPushButton *m_forwardButton;
    QPushButton *m_reloadButton;
    QPushButton *m_homeButton;
    QPushButton *m_copyButton;

    void setupUI();
    void setupConnections();
    void updateButtonStates();
    QPushButton *m_addWebsiteButton;
    QPushButton *m_openTabButton;
    QComboBox *m_searchEngineCombo;
};

#endif // URLBAR_H
