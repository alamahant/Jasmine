#ifndef MYWEBPAGE_H
#define MYWEBPAGE_H

#include <QWebEnginePage>

#include <QWebEngineView>
#include <QWebEngineProfile>

class MyWebPage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit MyWebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    QWebEnginePage* createWindow(WebWindowType type) override;
private:
    void connectStandardSignals();

signals:
    void newTabRequested(QWebEngineView *view, QWebEngineProfile *profile);
    void newPopupRequested(QWebEngineView* view, QWebEngineProfile* profile);  // New signal for popups
    void urlChangedExternally(const QUrl &url);
    void titleChangedExternally(const QString &title);
    void loadStartedExternally();
    void loadFinishedExternally(bool ok);
};

#endif // MYWEBPAGE_H
