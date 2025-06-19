#include "simpleadblocker.h"

SimpleAdBlocker::SimpleAdBlocker(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
    , m_enabled(true)
{
    // Common ad domains
    /*
    m_adDomains = {
        "googleads", "doubleclick.net", "adservice", "adsystem",
        "adtech", "advertising", "banners", "popunder", "popads",
        "adskeeper", "adnxs", "adroll", "admob", "scorecardresearch",
        "googlesyndication", "ad.doubleclick.net", "pagead",
        "adserver", "adnetwork", "admarketplace", "adform",
        "adzerk", "bidswitch", "buysellads", "carbonads",
        "casalemedia", "criteo", "ezoic", "medianet",
        "moatads", "outbrain", "pubmatic", "revcontent",
        "rubiconproject", "taboola", "teads", "zedo"
    };
    */

    m_adDomains = {
        // My existing domains
        "googleads", "doubleclick.net", "adservice", "adsystem",
        "adtech", "advertising", "banners", "popunder", "popads",
        "adskeeper", "adnxs", "adroll", "admob", "scorecardresearch",
        "googlesyndication", "ad.doubleclick.net", "pagead",
        "adserver", "adnetwork", "admarketplace", "adform",
        "adzerk", "bidswitch", "buysellads", "carbonads",
        "casalemedia", "criteo", "ezoic", "medianet",
        "moatads", "outbrain", "pubmatic", "revcontent",
        "rubiconproject", "taboola", "teads", "zedo",

        // Additional ad networks
        "2mdn.net", "2o7.net", "33across.com", "360yield.com", "4dsply.com",
        "adblade.com", "adbutler.com", "addthis.com", "adfox.ru", "adition.com",
        "adocean.pl", "adriver.ru", "adroll.com", "adsafeprotected.com", "adsrvr.org",
        "adswizz.com", "adtechus.com", "advertising.com", "advertserve.com", "affiliatly.com",
        "amazon-adsystem.com", "amplitude.com", "appier.net", "applovin.com", "appnexus.com",
        "atemda.com", "atdmt.com", "atwola.com", "automatad.com", "ayads.co",

        // More ad networks
        "baidu.com/cpro", "bannerflow.com", "bannersnack.com", "beachfront.com", "betweendigital.com",
        "bidvertiser.com", "bidr.io", "bkrtx.com", "bluekai.com", "bounceexchange.com",
        "brealtime.com", "brightroll.com", "btrll.com", "chartbeat.com", "chartboost.com",
        "clickagy.com", "clicktale.net", "cloudfront.net/atrk.gif", "comscore.com", "connatix.com",
        "contextweb.com", "conversantmedia.com", "cpmstar.com", "creative-serving.com", "crwdcntrl.net",

        // Tracking and analytics
        "demdex.net", "dotomi.com", "doublepimp.com", "doubleverify.com", "effectivemeasure.net",
        "emxdgt.com", "exelator.com", "exponential.com", "extreme-dm.com", "eyeota.net",
        "facebook.com/tr", "fiftyt.com", "flurry.com", "fwmrm.net", "gemius.pl",
        "geoedge.be", "google-analytics.com", "googletagmanager.com", "googletagservices.com", "gstatic.com/firebasejs",
        "gumgum.com", "heapanalytics.com", "histats.com", "hotjar.com", "hs-analytics.net",

        // More tracking
        "iasds01.com", "ibillboard.com", "indexww.com", "infolinks.com", "innovid.com",
        "insightexpressai.com", "instana.io", "intellitxt.com", "intercom.io", "intergi.com",
        "iplocationtools.com", "iponweb.com", "irs01.com", "juicyads.com", "justpremium.com",
        "keywee.co", "kissmetrics.com", "krxd.net", "lijit.com", "linkedin.com/px",
        "lkqd.net", "loopme.me", "marfeel.com", "marketo.net", "mathtag.com",

        // Additional networks
        "media.net", "media6degrees.com", "mediaplex.com", "mgid.com", "mixpanel.com",
        "moengage.com", "molocoads.com", "mookie1.com", "mopub.com", "mouseflow.com",
        "nativeads.com", "netmng.com", "newrelic.com", "nexac.com", "npttech.com",
        "nr-data.net", "nuggad.net", "omniture.com", "omtrdc.net", "onespot.com",
        "openx.net", "optimizely.com", "owneriq.net", "pardot.com", "parsely.com",

        // More networks
        "perfectmarket.com", "petametrics.com", "pixfuture.com", "playground.xyz", "plista.com",
        "polar.me", "polyfill.io", "postrelease.com", "prebid.org", "pressboard.ca",
        "prfct.co", "pro-market.net", "proper.io", "pswec.com", "pulsepoint.com",
        "pusher.com", "pusherapp.com", "quantcast.com", "quantserve.com", "quantum-advertising.com",
        "quora.com/qevents", "rambler.ru", "rhythmone.com", "richaudience.com", "rlcdn.com",

        // Final batch
        "rtbhouse.com", "rtk.io", "rubicon.com", "sail-horizon.com", "samba.tv",
        "sascdn.com", "scarabresearch.com", "scorecardresearch.com", "segment.com", "segment.io",
        "sekindo.com", "serving-sys.com", "sharethrough.com", "sharethis.com", "simpli.fi",
        "sitemeter.com", "skimresources.com", "smaato.net", "smartadserver.com", "smartclip.net",
        "snapchat.com/activity", "sonobi.com", "spotxchange.com", "springserve.com", "stackadapt.com",
        "statcounter.com", "stickyadstv.com", "stormiq.com", "sumome.com", "sumo.com",
        "tapad.com", "tealiumiq.com", "theadex.com", "themoneytizer.com", "tiqcdn.com",
        "tns-counter.ru", "traffichunt.com", "tribalfusion.com", "triplelift.com", "truoptik.com",
        "turn.com", "tynt.com", "undertone.com", "unrulymedia.com", "usabilla.com",
        "viglink.com", "vilynx.com", "visualwebsiteoptimizer.com", "vizu.com", "voicefive.com",
        "w55c.net", "webtrends.com", "weborama.fr", "widespace.com", "xiti.com",
        "yandex.ru/metrika", "yieldlab.net", "yieldmo.com", "yieldoptimizer.com", "yotpo.com",
        "youbora.com", "zendesk.com", "zergnet.com", "zorosrv.com"
    };


}

void SimpleAdBlocker::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    if (!m_enabled) return;

    QUrl url = info.requestUrl();
    QString urlString = url.toString();
    QString host = url.host();

    // Check if the URL contains any ad domain
    for (const QString &adDomain : m_adDomains) {
        if (host.contains(adDomain) || urlString.contains(adDomain)) {
            info.block(true);
            return;
        }
    }
}

