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

    // Add raw JS or CSS as a script
    void addScript(const QString &name, const QString &sourceCode,
                   QWebEngineScript::InjectionPoint point = QWebEngineScript::DocumentReady);

    // Convenience: add pure CSS rules
    void addCssFilter(const QString &name, const QString &css);

private:
    QWebEngineProfile *m_profile;

    // Install default cosmetic filters (YouTube, Facebook, etc.)
    void installDefaultFilters();
};

#endif // COSMETICFILTERINJECTOR_H
