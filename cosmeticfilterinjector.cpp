#include "cosmeticfilterinjector.h"
#include <QWebEngineScriptCollection>

CosmeticFilterInjector::CosmeticFilterInjector(QWebEngineProfile *profile, QObject *parent)
    : QObject(parent), m_profile(profile)
{
    installDefaultFilters(); // auto-install filters on creation
}

void CosmeticFilterInjector::addScript(const QString &name, const QString &sourceCode,
                                       QWebEngineScript::InjectionPoint point)
{
    QWebEngineScript script;
    script.setName(name);
    script.setInjectionPoint(point);
    script.setRunsOnSubFrames(true);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode(sourceCode);
    m_profile->scripts()->insert(script);
}

void CosmeticFilterInjector::addCssFilter(const QString &name, const QString &css)
{
    QString js = QString(R"(
        (function() {
            var style = document.createElement('style');
            style.textContent = `%1`;
            document.documentElement.appendChild(style);
        })();
    )").arg(css);
    addScript(name, js, QWebEngineScript::DocumentReady);
}
/*
void CosmeticFilterInjector::installDefaultFilters()
{
    // Hide YouTube ads
    addCssFilter("YouTubeAds", R"CSS(
        .ytp-ad-module,
        .ytp-ad-player-overlay,
        .ytp-ad-image-overlay,
        .video-ads,
        .ytp-ad-overlay-container {
            display: none !important;
        }
    )CSS");

    // Hide Facebook sponsored posts
    addCssFilter("FacebookSponsored", R"CSS(
        div[aria-label="Sponsored"],
        a[href*="facebook.com/ads"] {
            display: none !important;
        }
    )CSS");
}
*/


////recent edits
void CosmeticFilterInjector::installDefaultFilters()
{
    // YouTube ad blocker with MutationObserver
    QString youtubeScript = R"JS(
        (function() {
            // CSS to hide ads
            const style = document.createElement('style');
            style.textContent = `
                .ytp-ad-module,
                .ytp-ad-player-overlay,
                .ytp-ad-image-overlay,
                .ytp-ad-text-overlay,
                .video-ads,
                .ytp-ad-overlay-container,
                ytd-display-ad-renderer,
                ytd-promoted-sparkles-web-renderer,
                #masthead-ad,
                .ytd-compact-promoted-item-renderer {
                    display: none !important;
                    visibility: hidden !important;
                }
            `;
            (document.head || document.documentElement).appendChild(style);

            // Function to remove ad elements
            function removeAds() {
                // Remove video ads
                const adSelectors = [
                    '.ytp-ad-module',
                    '.ytp-ad-player-overlay',
                    '.video-ads',
                    'ytd-display-ad-renderer',
                    'ytd-promoted-sparkles-web-renderer'
                ];

                adSelectors.forEach(selector => {
                    document.querySelectorAll(selector).forEach(el => el.remove());
                });

                // Auto-skip ads if skip button appears
                const skipButton = document.querySelector('.ytp-ad-skip-button, .ytp-skip-ad-button');
                if (skipButton) {
                    skipButton.click();
                }
            }

            // Run immediately
            removeAds();

            // Watch for dynamically added ads
            const observer = new MutationObserver(removeAds);
            observer.observe(document.documentElement, {
                childList: true,
                subtree: true
            });

            // Also run periodically as backup
            setInterval(removeAds, 1000);
        })();
    )JS";

    addScript("YouTubeAdBlocker", youtubeScript, QWebEngineScript::DocumentCreation);

    // Facebook sponsored posts
    QString facebookScript = R"JS(
        (function() {
            const style = document.createElement('style');
            style.textContent = `
                div[aria-label="Sponsored"],
                a[href*="facebook.com/ads"],
                div[data-pagelet*="FeedUnit"]:has(span:contains("Sponsored")) {
                    display: none !important;
                }
            `;
            (document.head || document.documentElement).appendChild(style);

            function removeSponsored() {
                // Remove sponsored posts
                document.querySelectorAll('div[aria-label="Sponsored"]').forEach(el => {
                    el.closest('div[data-pagelet]')?.remove();
                });
            }

            const observer = new MutationObserver(removeSponsored);
            observer.observe(document.body || document.documentElement, {
                childList: true,
                subtree: true
            });

            setInterval(removeSponsored, 2000);
        })();
    )JS";

    addScript("FacebookAdBlocker", facebookScript, QWebEngineScript::DocumentCreation);
}
