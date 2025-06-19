#ifndef SIMPLEADBLOCKER_H
#define SIMPLEADBLOCKER_H

#include <QWebEngineUrlRequestInterceptor>
#include <QSet>
#include <QDebug>

class SimpleAdBlocker : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    explicit SimpleAdBlocker(QObject *parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    QSet<QString> m_adDomains;
    bool m_enabled;
};

#endif // SIMPLEADBLOCKER_H

