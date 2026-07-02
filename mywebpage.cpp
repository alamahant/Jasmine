#include "mywebpage.h"
#include <QDebug>
#include<QTimer>

MyWebPage::MyWebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    connectStandardSignals();
    /*
    connect(this, &QWebEnginePage::urlChanged, [this](const QUrl &url) {
        if (url.host().contains("youtube.com") || url.host().contains("youtu.be")) {
            QTimer::singleShot(1500, [this, url]() {
                if (this->url() == url && !this->title().isEmpty()) {
                    emit loadFinished(true);
                }
            });
        }
    });
    */

}

void MyWebPage::connectStandardSignals() {
    connect(this, &QWebEnginePage::urlChanged, this, &MyWebPage::urlChangedExternally);
    connect(this, &QWebEnginePage::titleChanged, this, &MyWebPage::titleChangedExternally);
    connect(this, &QWebEnginePage::loadStarted, this, &MyWebPage::loadStartedExternally);
    connect(this, &QWebEnginePage::loadFinished, this, &MyWebPage::loadFinishedExternally);
}


QWebEnginePage* MyWebPage::createWindow(WebWindowType type) {
    Q_UNUSED(type);

    QWebEngineView *newView = new QWebEngineView();

    MyWebPage *newPage = new MyWebPage(this->profile(), newView);
    newView->setPage(newPage);


    QWebEngineProfile* callingTabProfile = this->profile();
    connect(newPage, &QWebEnginePage::urlChanged, this,
        [this, newView, newPage, callingTabProfile](const QUrl &url){
            if (!url.isEmpty()) {


                //emit newPopupRequested(newView, this->profile());
                emit newPopupRequested(newView, callingTabProfile);

                disconnect(newPage, &QWebEnginePage::urlChanged, this, nullptr);
            }
        });

    return newPage;
}


