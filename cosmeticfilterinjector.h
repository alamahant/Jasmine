#ifndef COSMETICFILTERINJECTOR_H
#define COSMETICFILTERINJECTOR_H

#include <QObject>

#include <QWebEngineProfile>
#include <QWebEngineScript>

#include <QWebEngineProfile>
#include <QWebEngineScript>

class CosmeticFilterInjector : public QObject
{
    Q_OBJECT
public:
    explicit CosmeticFilterInjector(QWebEngineProfile *profile, QObject *parent = nullptr);

    void addScript(const QString &name, const QString &sourceCode,
                   QWebEngineScript::InjectionPoint point = QWebEngineScript::DocumentReady);

    void addCssFilter(const QString &name, const QString &css);

private:
    QWebEngineProfile *m_profile;

    void installDefaultFilters();
};

#endif // COSMETICFILTERINJECTOR_H
