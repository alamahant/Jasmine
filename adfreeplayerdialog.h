#ifndef ADFREEPLAYERDIALOG_H
#define ADFREEPLAYERDIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QProcess>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

class AdFreePlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdFreePlayerDialog(QWidget *parent = nullptr);
    ~AdFreePlayerDialog();

    void setUrl(const QString &url);
protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void requestCurrentUrl();

private slots:
    void onGetUrlClicked();
    void playStream();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void onPlayPause();
    void onStop();
    void onVolumeChanged(int value);
    void onProgressSliderMoved(int value);
    void toggleFullscreen();
    void showContextMenu(const QPoint &pos);
    void onClearClicked();
private:
    void setupUI();
    void createVideoToolbar();
    void extractStreamUrl();
    QString truncateUrl(const QString &url, int maxChars);

    QString m_currentUrl;
    QMediaPlayer *m_player;
    QVideoWidget *m_videoWidget;
    QProcess *m_ytProcess;

    // UI elements
    QPushButton *m_getUrlButton;
    QLabel *m_urlDisplay;
    QWidget *m_videoToolbar;
    QPushButton *m_playButton;
    QPushButton *m_stopButton;
    QSlider *m_volumeSlider;
    QSlider *m_progressSlider;
    QLabel *m_timeLabel;
    QPushButton *m_fullscreenButton;

    bool m_isPlaying;
    int m_lastVolume = 70;
    QWidget *topBar;
    QPushButton *m_clearButton;
    void playStreamWithUrl(const QString &streamUrl);
    QByteArray m_outputBuffer;
};

#endif
