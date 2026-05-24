#ifndef SEARCHRADIOSTATIONSDIALOG_H
#define SEARCHRADIOSTATIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "radiobrowserapi.h"
#include "radiostation.h"


class SearchRadioStationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchRadioStationsDialog(QWidget *parent = nullptr);
    ~SearchRadioStationsDialog();

    RadioBrowserAPI *api() const;

signals:
    void stationSelected(const RadioStation &station);
    void showNotification(const QString& msg, int duration);
    void previewRadioStation(const QString &streamUrl, const QString &name);
private slots:
    void onSearchClicked();
    void onSearchResult(const QJsonArray &stations);
    void onResultSelectionChanged();
    void onPlayClicked();
    void onAddClicked();
    void onApiError(const QString &message);
private:
    void setupUI();
    void performSearch();
    void populateCountryCombo();
    RadioStation parseJsonToStation(const QJsonObject &obj);
    void updatePreview(const RadioStation &station);
    void clearPreview();

    // UI Components
    QComboBox *m_countryCombo;
    QLineEdit *m_nameEdit;
    QPushButton *m_searchButton;
    QTableWidget *m_resultsTable;
    
    // Preview section
    QLabel *m_previewName;
    QLabel *m_previewUrl;
    QLabel *m_previewCountry;
    QLabel *m_previewBitrate;
    QLabel *m_previewCodec;
    QLabel *m_previewGenre;
    
    // Buttons
    QPushButton *m_playButton;
    QPushButton *m_addButton;
    QPushButton *m_cancelButton;
    
    // API and data
    RadioBrowserAPI *m_api;
    QVector<RadioStation> m_currentResults;
    int m_currentSelectedIndex;
private:
    QLineEdit *m_filterEdit;  // Filter/search within results
    void filterResults(const QString &text);
};

#endif // SEARCHRADIOSTATIONSDIALOG_H
