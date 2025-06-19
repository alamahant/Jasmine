#ifndef ADBLOCKSCRIPT_H
#define ADBLOCKSCRIPT_H
#include <QString>

// Simple class to provide ad-blocking JavaScript
class AdBlockScript
{
public:
    static QString getScript()
    {
        // More comprehensive CSS rules
        QString customCssRules = R"(
            /* Common ad selectors */
            div[id*="google_ads"],
            ins.adsbygoogle,
            div[id*="banner"],
            div[class*="ad-container"],
            div[class*="ad_container"],
            div[class*="advert"],
            iframe[src*="doubleclick"],
            iframe[src*="googleads"],
            iframe[id*="google_ads"],
            img[src*="ads"],
            div[id*="div-gpt-ad"],
            .advertisement,
            .ad-unit,
            .sponsored-content,
            .sponsored-result,
            div[data-ad],
            div[data-adunit],
            div[data-adslot],
            div[data-ad-slot],
            div[class*="adthrive"],
            div[class*="mgid"],
            aside[class*="sidebar"] div[class*="ad"],
            div[class*="trc_"],
            div[class*="taboola"],
            div[id*="taboola"],
            div[id*="outbrain"],
            div[class*="outbrain"],
            div[class*="OUTBRAIN"],
            div[class*="AdSlot"],
            div[class*="AdContainer"],
            div[class*="gemini-ad"],
            div[class*="ad-banner"],
            div[class*="ad-wrapper"],
            div[class*="adWrapper"] {
                display: none !important;
                opacity: 0 !important;
                pointer-events: none !important;
                height: 0 !important;
                position: absolute !important;
                z-index: -1 !important;
            }
        )";

        // Build the full script with the CSS rules
        return R"(
            (function() {
                // Apply CSS rules to block ads
                function addStyleSheet() {
                    const style = document.createElement('style');
                    style.textContent = `)" + customCssRules + R"(`;
                    style.id = 'ad-blocker-style';
                    (document.head || document.documentElement).appendChild(style);
                }

                // Remove existing ads
                function removeAds() {
                    // Find ad iframes
                    const iframes = document.querySelectorAll('iframe');
                    for (let i = 0; i < iframes.length; i++) {
                        const src = iframes[i].src || '';
                        if (src.includes('doubleclick') ||
                            src.includes('googleads') ||
                            src.includes('adsystem') ||
                            src.includes('adserver') ||
                            src.includes('adnxs') ||
                            src.includes('criteo')) {
                            iframes[i].style.display = 'none';
                        }
                    }

                    // Find ad scripts
                    const scripts = document.querySelectorAll('script');
                    for (let i = 0; i < scripts.length; i++) {
                        const src = scripts[i].src || '';
                        if (src.includes('adsbygoogle') ||
                            src.includes('pagead') ||
                            src.includes('scorecardresearch') ||
                            src.includes('googletagmanager') ||
                            src.includes('googletagservices')) {
                            scripts[i].parentNode.removeChild(scripts[i]);
                        }
                    }
                }

                // Block ad-related JavaScript variables
                function blockAdVars() {
                    // Override common ad-related globals
                    window.canRunAds = true;  // Trick anti-adblock
                    window.adsbygoogle = [];
                    window.googletag = {
                        cmd: [],
                        pubads: function() {
                            return {
                                refresh: function() {},
                                enableSingleRequest: function() {},
                                setTargeting: function() {},
                                addEventListener: function() {}
                            };
                        }
                    };

                    // Intercept ad-related functions
                    if (window.open) {
                        const originalOpen = window.open;
                        window.open = function(url, name, specs) {
                            if (url && typeof url === 'string' &&
                                (url.includes('ad') || url.includes('pop') ||
                                 url.includes('bet') || url.includes('casino'))) {
                                console.log('Blocked popup:', url);
                                return null;
                            }
                            return originalOpen.apply(this, arguments);
                        };
                    }
                }

                // Watch for dynamic ad insertions
                function watchForAds() {
                    const observer = new MutationObserver(function(mutations) {
                        removeAds();
                    });

                    observer.observe(document.body, {
                        childList: true,
                        subtree: true
                    });
                }

                // Run immediately
                addStyleSheet();
                removeAds();
                blockAdVars();

                // Run again when DOM is fully loaded
                if (document.readyState === 'loading') {
                    document.addEventListener('DOMContentLoaded', function() {
                        removeAds();
                        watchForAds();
                    });
                } else {
                    watchForAds();
                }

                // Run again when page is fully loaded
                window.addEventListener('load', function() {
                    removeAds();
                });
            })();
        )";
    }

};
#endif // ADBLOCKSCRIPT_H
