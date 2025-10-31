#ifndef MYWEBPAGE_H
#define MYWEBPAGE_H

#include <QWebEnginePage>

#include <QWebEngineView>
#include <QWebEngineProfile>

class MyWebPage : public QWebEnginePage {
    Q_OBJECT
public:
    // Match your existing code: parent is the QWebEngineView
    explicit MyWebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    // Handle new-window/tab requests
    QWebEnginePage* createWindow(WebWindowType type) override;

private:
    void connectStandardSignals();

signals:
    // Emitted when a new tab should be created
    void newTabRequested(QWebEngineView *view, QWebEngineProfile *profile);

    // Forwarded signals
    void urlChangedExternally(const QUrl &url);
    void titleChangedExternally(const QString &title);
    void loadStartedExternally();
    void loadFinishedExternally(bool ok);
};

#endif // MYWEBPAGE_H
