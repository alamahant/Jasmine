#include "adfreeplayerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAudioOutput>
#include<QCloseEvent>
#include<QMessageBox>
#include<QTimer>
#include<QRegularExpression>

AdFreePlayerDialog::AdFreePlayerDialog(QWidget *parent)
    : QDialog(parent)
    , m_player(nullptr)
    , m_videoWidget(nullptr)
    , m_ytProcess(nullptr)
    , m_isPlaying(false)
{
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowTitle("Jasmine - Ad-Free Player");
    setMinimumSize(800, 450);
    setupUI();
}

AdFreePlayerDialog::~AdFreePlayerDialog()
{
    if (m_player) m_player->stop();
    if (m_ytProcess) m_ytProcess->kill();
}

void AdFreePlayerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Video widget
    m_videoWidget = new QVideoWidget(this);

    // Install event filter on video widget and container
    m_videoWidget->installEventFilter(this);
    m_videoWidget->setAttribute(Qt::WA_AcceptTouchEvents);

    m_videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_videoWidget, &QVideoWidget::customContextMenuRequested, this, &AdFreePlayerDialog::showContextMenu);
    mainLayout->addWidget(m_videoWidget);

    // Top bar with URL and buttons
    topBar = new QWidget(this);
    topBar->setStyleSheet("background-color: rgba(0, 0, 0, 180); color: white;");
    topBar->setFixedHeight(45);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    m_getUrlButton = new QPushButton("Get URL", topBar);
    m_getUrlButton->setFixedWidth(160);

    m_urlDisplay = new QLabel("No URL loaded", topBar);
    m_urlDisplay->setStyleSheet("color: white; padding: 5px; background-color: rgba(0, 0, 0, 100); border-radius: 3px;");
    m_urlDisplay->setMinimumWidth(300);

    m_clearButton = new QPushButton("✖", topBar);  // Small clear button
    m_clearButton->setFixedSize(30, 30);
    m_clearButton->setToolTip("Clear URL and kill yt-dlp");
    m_clearButton->setStyleSheet("QPushButton { background-color: rgba(200, 60, 60, 150); border-radius: 15px; font-size: 16px; }"
                                  "QPushButton:hover { background-color: rgba(220, 80, 80, 200); }");

    topLayout->addWidget(m_getUrlButton);
    topLayout->addWidget(m_urlDisplay, 1);
    topLayout->addWidget(m_clearButton);

    mainLayout->addWidget(topBar);

    // Video toolbar (no next/previous buttons)
    createVideoToolbar();
    mainLayout->addWidget(m_videoToolbar);

    // Connect signals
    connect(m_getUrlButton, &QPushButton::clicked, this, &AdFreePlayerDialog::onGetUrlClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &AdFreePlayerDialog::onClearClicked);
    connect(m_playButton, &QPushButton::clicked, this, &AdFreePlayerDialog::onPlayPause);
    connect(m_stopButton, &QPushButton::clicked, this, &AdFreePlayerDialog::onStop);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AdFreePlayerDialog::onVolumeChanged);
    connect(m_progressSlider, &QSlider::sliderMoved, this, &AdFreePlayerDialog::onProgressSliderMoved);
    connect(m_fullscreenButton, &QPushButton::clicked, this, &AdFreePlayerDialog::toggleFullscreen);
}

void AdFreePlayerDialog::createVideoToolbar()
{
    m_videoToolbar = new QWidget();
    m_videoToolbar->setObjectName("videoToolbar");
    m_videoToolbar->setFixedHeight(50);

    QHBoxLayout *toolbarLayout = new QHBoxLayout(m_videoToolbar);
    toolbarLayout->setContentsMargins(10, 5, 10, 5);
    toolbarLayout->setSpacing(10);

    m_playButton = new QPushButton();
    m_playButton->setIcon(QIcon(":/resources/icons-white/play.svg"));
    m_playButton->setFixedSize(32, 32);
    m_playButton->setToolTip("Play");

    m_stopButton = new QPushButton();
    m_stopButton->setIcon(QIcon(":/resources/icons-white/square.svg"));
    m_stopButton->setFixedSize(32, 32);
    m_stopButton->setToolTip("Stop");

    QLabel *volLabel = new QLabel("Vol:");

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setMinimumWidth(100);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);

    QLabel *volValueLabel = new QLabel("70%");
    connect(m_volumeSlider, &QSlider::valueChanged, volValueLabel, [volValueLabel](int v) {
        volValueLabel->setText(QString("%1%").arg(v));
    });

    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setValue(0);

    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setStyleSheet("color: white; font-size: 12px;");
    m_timeLabel->setMinimumWidth(100);

    m_fullscreenButton = new QPushButton();
    m_fullscreenButton->setIcon(QIcon(":/resources/icons-white/maximize-2.svg"));
    m_fullscreenButton->setFixedSize(32, 32);
    m_fullscreenButton->setToolTip("Fullscreen");

    toolbarLayout->addWidget(m_playButton);
    toolbarLayout->addWidget(m_stopButton);
    toolbarLayout->addWidget(volLabel);
    toolbarLayout->addWidget(m_volumeSlider);
    toolbarLayout->addWidget(volValueLabel);
    toolbarLayout->addWidget(m_progressSlider, 1);
    toolbarLayout->addWidget(m_timeLabel);
    toolbarLayout->addWidget(m_fullscreenButton);

    QString toolbarStyle =
        "#videoToolbar {"
        "    background-color: rgba(0, 0, 0, 220);"
        "    border-top: 1px solid rgba(255, 255, 255, 50);"
        "    padding: 2px 8px;"
        "    spacing: 4px;"
        "}"
        "#videoToolbar QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "}"
        "#videoToolbar QPushButton {"
        "    color: white;"
        "    background-color: rgba(255, 255, 255, 30);"
        "    border: none;"
        "    border-radius: 3px;"
        "    padding: 4px 8px;"
        "    font-size: 13px;"
        "    min-height: 24px;"
        "}"
        "#videoToolbar QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 50);"
        "}"
        "#videoToolbar QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 70);"
        "}"
        "#videoToolbar QSlider {"
        "    height: 18px;"
        "}"
        "#videoToolbar QSlider::groove:horizontal {"
        "    height: 3px;"
        "    background: rgba(255, 255, 255, 50);"
        "    border-radius: 1.5px;"
        "}"
        "#videoToolbar QSlider::handle:horizontal {"
        "    background: white;"
        "    width: 10px;"
        "    height: 10px;"
        "    margin: -3.5px 0;"
        "    border-radius: 5px;"
        "}";

    m_videoToolbar->setStyleSheet(toolbarStyle);
}

void AdFreePlayerDialog::setUrl(const QString &url)
{
    m_currentUrl = url;
    m_urlDisplay->setText(url);
    m_urlDisplay->setStyleSheet("color: white; padding: 5px; background-color: rgba(0, 0, 0, 100); border-radius: 3px;");
    extractStreamUrl();
}

void AdFreePlayerDialog::onGetUrlClicked()
{
    // Check if a URL is already loaded
    if (!m_currentUrl.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Load New URL",
            "A video is already loaded.\nLoad new URL and replace current video?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (reply == QMessageBox::No) {
            return; // Cancel - don't load new URL
        }

        // User chose Yes - stop current playback
        if (m_player) {
            m_player->stop();
            m_isPlaying = false;
            m_playButton->setIcon(QIcon(":/icons-white/play.svg"));
            m_progressSlider->setValue(0);
            m_timeLabel->setText("00:00 / 00:00");
        }
    }

    emit requestCurrentUrl();
}

/*
void AdFreePlayerDialog::extractStreamUrl()
{
    if (m_currentUrl.isEmpty()) return;

    m_getUrlButton->setEnabled(false);
    m_getUrlButton->setText("Extracting...");

    m_ytProcess = new QProcess(this);

    QStringList args;
    // YouTube needs -f best for single merged URL
    if (m_currentUrl.contains("youtube.com") || m_currentUrl.contains("youtu.be") || m_currentUrl.contains("music.youtube")) {
        args << "-f" << "best" << "--no-progress" << "-g" << m_currentUrl;
    }
    // Vimeo also needs -f best for direct MP4
    else if (m_currentUrl.contains("vimeo.com")) {
        args << "-f" << "best" << "-g" << m_currentUrl;
    }
    // Dailymotion, Rumble, Odysee work with just -g
    else {
        args << "-g" << m_currentUrl;
    }

    connect(m_ytProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AdFreePlayerDialog::playStream);

    m_ytProcess->start("yt-dlp", args);
}
*/


void AdFreePlayerDialog::extractStreamUrl()
{
    if (m_currentUrl.isEmpty()) return;

    m_getUrlButton->setEnabled(false);
    m_getUrlButton->setText("Extracting...");

    // Clean up previous process if exists
    if (m_ytProcess) {
        disconnect(m_ytProcess, nullptr, this, nullptr);
        if (m_ytProcess->state() == QProcess::Running) {
            m_ytProcess->kill();
        }
        m_ytProcess->deleteLater();
        m_ytProcess = nullptr;
    }

    m_ytProcess = new QProcess(this);
    m_outputBuffer.clear();

    QStringList args;

    QString userAgent =
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/136.0 Safari/537.36";

    //args << "--user-agent" << userAgent;

    //if (m_currentUrl.contains("youtube.com") || m_currentUrl.contains("youtu.be") || m_currentUrl.contains("music.youtube.com")) {
        args << "--no-progress" << "--no-playlist" << "--quiet" <<  "-f" << "best" << "-g" << m_currentUrl;
    //} else if (m_currentUrl.contains("vimeo.com")) {
      //  args << "-f" << "best" << "-g" << m_currentUrl;
    //} else {
      //  args << "-g" << m_currentUrl;
    //}



    // Collect output as it comes
    connect(m_ytProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        m_outputBuffer.append(m_ytProcess->readAllStandardOutput());
        QString output = QString::fromUtf8(m_outputBuffer);
        qDebug() << "URL " << output;

    });

    // Single timer - just wait 9 seconds then get URL
    QTimer::singleShot(9000, this, [this]() {
        if (!m_ytProcess) return;

        // Kill the process if still running
        if (m_ytProcess->state() == QProcess::Running) {
            m_ytProcess->kill();
        }

        QString output = QString::fromUtf8(m_outputBuffer);
        qDebug() << "URL " << output;

        QStringList lines = output.split('\n');

        QString streamUrl;
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.startsWith("http://") || trimmed.startsWith("https://")) {
                streamUrl = trimmed;
                break;
            }
        }

        qDebug() << "URL " << streamUrl;

        if (!streamUrl.isEmpty()) {
            playStreamWithUrl(streamUrl);
        } else {
            m_getUrlButton->setEnabled(true);
            m_getUrlButton->setText("Get URL");
            m_urlDisplay->setText("Failed to extract stream");
        }

        m_ytProcess->deleteLater();
        m_ytProcess = nullptr;
    });


    connect(m_ytProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        m_getUrlButton->setEnabled(true);
        m_getUrlButton->setText("Get URL");
        m_urlDisplay->setText("Extraction error");
        if (m_ytProcess) {
            m_ytProcess->deleteLater();
            m_ytProcess = nullptr;
        }
    });


    m_ytProcess->start("yt-dlp", args);
}



void AdFreePlayerDialog::playStreamWithUrl(const QString &streamUrl)
{
    m_getUrlButton->setEnabled(true);
    m_getUrlButton->setText("Get URL");

    if (streamUrl.isEmpty()) {
        m_urlDisplay->setText("Failed to extract stream");
        return;
    }

    if (!m_player) {
        m_player = new QMediaPlayer(this);
        m_player->setVideoOutput(m_videoWidget);

        connect(m_player, &QMediaPlayer::positionChanged, this, &AdFreePlayerDialog::updatePosition);
        connect(m_player, &QMediaPlayer::durationChanged, this, &AdFreePlayerDialog::updateDuration);

        QAudioOutput *audioOutput = new QAudioOutput(this);
        m_player->setAudioOutput(audioOutput);
        audioOutput->setVolume(m_volumeSlider->value() / 100.0);
    }

    m_player->setSource(QUrl(streamUrl));
    m_player->play();

    m_isPlaying = true;
    m_playButton->setIcon(QIcon(":/resources/icons-white/pause.svg"));
    m_playButton->setToolTip("Pause");
    m_stopButton->setEnabled(true);
    m_progressSlider->setEnabled(true);
}

void AdFreePlayerDialog::playStream()
{
    m_getUrlButton->setEnabled(true);
    m_getUrlButton->setText("Get Current URL");

    if (!m_ytProcess || m_ytProcess->exitCode() != 0) {
        m_urlDisplay->setText("Failed to extract stream");
        return;
    }

    QString streamUrl = QString::fromUtf8(m_ytProcess->readAllStandardOutput()).trimmed();
    if (streamUrl.isEmpty()) return;

    if (!m_player) {
        m_player = new QMediaPlayer(this);
        m_player->setVideoOutput(m_videoWidget);

        connect(m_player, &QMediaPlayer::positionChanged, this, &AdFreePlayerDialog::updatePosition);
        connect(m_player, &QMediaPlayer::durationChanged, this, &AdFreePlayerDialog::updateDuration);

        QAudioOutput *audioOutput = new QAudioOutput(this);
        m_player->setAudioOutput(audioOutput);
        audioOutput->setVolume(m_volumeSlider->value() / 100.0);
    }

    m_player->setSource(QUrl(streamUrl));
    m_player->play();

    m_isPlaying = true;
    m_playButton->setIcon(QIcon(":/resources/icons-white/pause.svg"));
    m_playButton->setToolTip("Pause");
    m_stopButton->setEnabled(true);
    m_progressSlider->setEnabled(true);
}

void AdFreePlayerDialog::updatePosition(qint64 position)
{
    if (!m_progressSlider->isSliderDown() && m_player && m_player->duration() > 0) {
        m_progressSlider->setValue(position * 100 / m_player->duration());
    }

    int seconds = position / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;

    int totalSeconds = m_player ? m_player->duration() / 1000 : 0;
    int totalMinutes = totalSeconds / 60;
    totalSeconds = totalSeconds % 60;

    m_timeLabel->setText(QString("%1:%2 / %3:%4")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(totalMinutes, 2, 10, QChar('0'))
        .arg(totalSeconds, 2, 10, QChar('0')));
}

void AdFreePlayerDialog::updateDuration(qint64 duration)
{
    Q_UNUSED(duration);
}

void AdFreePlayerDialog::onPlayPause()
{
    if (!m_player) return;

    if (m_isPlaying) {
        m_player->pause();
        m_playButton->setIcon(QIcon(":/resources/icons-white/play.svg"));
        m_playButton->setToolTip("Play");
    } else {
        m_player->play();
        m_playButton->setIcon(QIcon(":/resources/icons-white/pause.svg"));
        m_playButton->setToolTip("Pause");
    }
    m_isPlaying = !m_isPlaying;
}

void AdFreePlayerDialog::onStop()
{
    if (m_player) {
        m_player->stop();
        m_isPlaying = false;
        m_playButton->setIcon(QIcon(":/resources/icons-white/play.svg"));
        m_playButton->setToolTip("Play");
        m_progressSlider->setValue(0);
        m_timeLabel->setText("00:00 / 00:00");
    }
}

void AdFreePlayerDialog::onVolumeChanged(int value)
{
    if (m_player && m_player->audioOutput()) {
        m_player->audioOutput()->setVolume(value / 100.0);
    }
}

void AdFreePlayerDialog::onProgressSliderMoved(int value)
{
    if (m_player && m_player->duration() > 0) {
        m_player->setPosition(value * m_player->duration() / 100);
    }
}

void AdFreePlayerDialog::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

QString AdFreePlayerDialog::truncateUrl(const QString &url, int maxChars)
{
    if (url.length() <= maxChars) return url;
    return url.left(maxChars - 3) + "...";
}

void AdFreePlayerDialog::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction *playAction = menu.addAction("Play");
    QAction *pauseAction = menu.addAction("Pause");
    QAction *stopAction = menu.addAction("Stop");
    menu.addSeparator();

    QAction *increaseVolumeAction = menu.addAction("Volume Up");
    QAction *decreaseVolumeAction = menu.addAction("Volume Down");
    QAction *muteAction = menu.addAction("Mute/Unmute");
    menu.addSeparator();

    QAction *fullscreenAction = menu.addAction(
        isFullScreen() ? "Exit Fullscreen" : "Fullscreen");
    menu.addSeparator();

    QMenu *aspectMenu = menu.addMenu("Aspect Ratio");
    QAction *actionKeepAspect = aspectMenu->addAction("Default");
    QAction *actionZoom = aspectMenu->addAction("Zoom");
    QAction *actionStretch = aspectMenu->addAction("Stretch");

    connect(actionKeepAspect, &QAction::triggered, this, [this]() {
        if (m_videoWidget) {
            m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
        }
    });

    connect(actionZoom, &QAction::triggered, this, [this]() {
        if (m_videoWidget) {
            m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        }
    });

    connect(actionStretch, &QAction::triggered, this, [this]() {
        if (m_videoWidget) {
            m_videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
        }
    });

    connect(playAction, &QAction::triggered, this, [this]() {
        if (m_player && m_player->playbackState() != QMediaPlayer::PlayingState) {
            m_player->play();
            m_isPlaying = true;
            m_playButton->setIcon(QIcon(":/resources/icons-white/pause.svg"));
        }
    });

    connect(pauseAction, &QAction::triggered, this, [this]() {
        if (m_player && m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->pause();
            m_isPlaying = false;
            m_playButton->setIcon(QIcon(":/resources/icons-white/play.svg"));
        }
    });

    connect(stopAction, &QAction::triggered, this, &AdFreePlayerDialog::onStop);

    connect(increaseVolumeAction, &QAction::triggered, this, [this]() {
        int newVolume = qMin(m_volumeSlider->value() + 10, 100);
        m_volumeSlider->setValue(newVolume);
    });

    connect(decreaseVolumeAction, &QAction::triggered, this, [this]() {
        int newVolume = qMax(m_volumeSlider->value() - 10, 0);
        m_volumeSlider->setValue(newVolume);
    });

    connect(muteAction, &QAction::triggered, this, [this]() {
        if (m_volumeSlider->value() > 0) {
            m_lastVolume = m_volumeSlider->value();
            m_volumeSlider->setValue(0);
        } else {
            m_volumeSlider->setValue(m_lastVolume);
        }
    });

    connect(fullscreenAction, &QAction::triggered, this, &AdFreePlayerDialog::toggleFullscreen);

    menu.exec(m_videoWidget->mapToGlobal(pos));
}

void AdFreePlayerDialog::closeEvent(QCloseEvent *event)
{
    // Stop playback
    if (m_player) {
        m_player->stop();
    }

    // Just hide the dialog, don't delete it
    hide();
    event->accept();
}

bool AdFreePlayerDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_videoWidget) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // Toggle both top bar and video toolbar
                bool topBarVisible = topBar->isVisible();
                bool toolbarVisible = m_videoToolbar->isVisible();

                if (topBarVisible || toolbarVisible) {
                    topBar->hide();
                    m_videoToolbar->hide();
                } else {
                    topBar->show();
                    m_videoToolbar->show();
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

void AdFreePlayerDialog::onClearClicked()
{

    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("Clear Current Video");
    confirmBox.setText("Are you sure you want to clear the current video?");
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);

    int result = confirmBox.exec();

    if (result != QMessageBox::Yes) {
        return; // User cancelled
    }

    // Kill yt-dlp process if running
    if (m_ytProcess && m_ytProcess->state() == QProcess::Running) {
        m_ytProcess->kill();
        m_ytProcess->waitForFinished(1000);
        m_ytProcess->deleteLater();
        m_ytProcess = nullptr;
    }

    // Stop playback
    if (m_player) {
        m_player->stop();
        m_isPlaying = false;
        m_playButton->setIcon(QIcon(":/resources/icons-white/play.svg"));
        m_progressSlider->setValue(0);
        m_timeLabel->setText("00:00 / 00:00");
    }

    // Clear URL display
    m_currentUrl.clear();
    m_urlDisplay->setText("No URL loaded");
    m_urlDisplay->setStyleSheet("color: white; padding: 5px; background-color: rgba(0, 0, 0, 100); border-radius: 3px;");

    // Re-enable get URL button if it was disabled
    m_getUrlButton->setEnabled(true);
    m_getUrlButton->setText("Get URL");

    //statusBar()->showMessage("Cleared", 2000); // If you have status bar, or just ignore
}
