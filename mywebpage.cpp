#include "mywebpage.h"
#include <QDebug>
#include<QTimer>

MyWebPage::MyWebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    connectStandardSignals();
}

// Forward key signals to MainWindow or tab manager
void MyWebPage::connectStandardSignals() {
    connect(this, &QWebEnginePage::urlChanged, this, &MyWebPage::urlChangedExternally);
    connect(this, &QWebEnginePage::titleChanged, this, &MyWebPage::titleChangedExternally);
    connect(this, &QWebEnginePage::loadStarted, this, &MyWebPage::loadStartedExternally);
    connect(this, &QWebEnginePage::loadFinished, this, &MyWebPage::loadFinishedExternally);
}

// Called when the page wants to open a new window/tab

QWebEnginePage* MyWebPage::createWindow(WebWindowType type) {
    Q_UNUSED(type);

    // Create the new view
    QWebEngineView *newView = new QWebEngineView();

    // Create a new MyWebPage with the same profile
    MyWebPage *newPage = new MyWebPage(this->profile(), newView);
    newView->setPage(newPage);

    // Emit signal to MainWindow for tab insertion

    QWebEngineProfile* callingTabProfile = this->profile();
    connect(newPage, &QWebEnginePage::urlChanged, this,
        [this, newView, newPage, callingTabProfile](const QUrl &url){
            if (!url.isEmpty()) {


                emit newPopupRequested(newView, this->profile());

                // Disconnect this specific connection
                disconnect(newPage, &QWebEnginePage::urlChanged, this, nullptr);
            }
        });

    //emit newTabRequested(newView, this->profile());
    return newPage;
}




