#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H

#include<QWebEngineUrlRequestInterceptor>
#include<QWebEngineUrlRequestInfo>



class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    RequestInterceptor(QObject *parent = nullptr) : QWebEngineUrlRequestInterceptor(parent) {}

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        info.setHttpHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
        info.setHttpHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
        info.setHttpHeader("Accept-Language", "en-US,en;q=0.9");
        info.setHttpHeader("Sec-Fetch-Dest", "document");
        info.setHttpHeader("Sec-Fetch-Mode", "navigate");
        info.setHttpHeader("Sec-Fetch-Site", "none");
    }
};
#endif // REQUESTINTERCEPTOR_H
