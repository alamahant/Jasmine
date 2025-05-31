#include "twofamanager.h"

#include "simple2fa.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QSettings>
#include <QProgressBar>
#include <QGroupBox>
#include <QSplitter>

TwoFAManager::TwoFAManager(QWidget *parent)
    :QWidget(nullptr, Qt::Window)

    , m_accountList(nullptr)
    , m_currentCodeLabel(nullptr)
    , m_timeRemainingLabel(nullptr)
    , m_deleteBtn(nullptr)
    , m_copyBtn(nullptr)
    , m_updateTimer(nullptr)
    , m_timeProgressBar(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("2FA Code Generator");
    setWindowIcon(QIcon(":/resources/2fa-icon.png"));
    resize(450, 350);

    setupUI();
    loadData();
    // Force reset progress bar on startup
    m_timeProgressBar->setValue(Simple2FA::getTimeRemaining());
    // Start the update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TwoFAManager::updateCurrentCode);
    m_updateTimer->start(1000); // Update every second

    updateCurrentCode();
}

TwoFAManager::~TwoFAManager()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    saveData();
}

void TwoFAManager::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create splitter for resizable sections
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    // === ACCOUNTS SECTION ===
    QGroupBox* accountsGroup = new QGroupBox("2FA Accounts");
    QVBoxLayout* accountsLayout = new QVBoxLayout(accountsGroup);

    m_accountList = new QListWidget;
    m_accountList->setMinimumHeight(150);
    accountsLayout->addWidget(m_accountList);

    // Account buttons
    QHBoxLayout* accountButtonsLayout = new QHBoxLayout;
    QPushButton* addBtn = new QPushButton("Add Account");
    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setEnabled(false);

    accountButtonsLayout->addWidget(addBtn);
    accountButtonsLayout->addWidget(m_deleteBtn);
    accountButtonsLayout->addStretch();
    accountsLayout->addLayout(accountButtonsLayout);

    splitter->addWidget(accountsGroup);

    // === CODE DISPLAY SECTION ===
    QGroupBox* codeGroup = new QGroupBox("Current Code");
    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);

    // Code display frame
    QFrame* codeFrame = new QFrame;
    codeFrame->setFrameStyle(QFrame::StyledPanel);
    codeFrame->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border-radius: 8px; padding: 10px; }"
        );
    QVBoxLayout* codeFrameLayout = new QVBoxLayout(codeFrame);

    m_currentCodeLabel = new QLabel("------");
    m_currentCodeLabel->setAlignment(Qt::AlignCenter);
    m_currentCodeLabel->setStyleSheet(
        "font-size: 32px; font-weight: bold; color: #2196F3; "
        "font-family: 'Courier New', monospace; letter-spacing: 4px;"
        );
    codeFrameLayout->addWidget(m_currentCodeLabel);

    // Time remaining
    m_timeRemainingLabel = new QLabel("Select an account");
    m_timeRemainingLabel->setAlignment(Qt::AlignCenter);
    m_timeRemainingLabel->setStyleSheet("font-size: 12px; color: #666;");
    codeFrameLayout->addWidget(m_timeRemainingLabel);

    // Progress bar for time remaining
    m_timeProgressBar = new QProgressBar;
    m_timeProgressBar->setRange(0, 30);
    m_timeProgressBar->setValue(30);
    m_timeProgressBar->setTextVisible(false);
    m_timeProgressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #ddd; border-radius: 4px; background: #f0f0f0; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #4CAF50, stop:0.7 #FFC107, stop:1 #F44336); border-radius: 3px; }"
        );
    codeFrameLayout->addWidget(m_timeProgressBar);

    codeLayout->addWidget(codeFrame);

    // Copy button
    m_copyBtn = new QPushButton("Copy Code to Clipboard");
    m_copyBtn->setEnabled(false);
    m_copyBtn->setStyleSheet(
        "QPushButton { font-weight: bold; padding: 8px; }"
        "QPushButton:enabled { background-color: #4CAF50; color: white; }"
        );
    codeLayout->addWidget(m_copyBtn);

    splitter->addWidget(codeGroup);
    splitter->setSizes({200, 150});

    mainLayout->addWidget(splitter);

    // === CONNECTIONS ===
    connect(addBtn, &QPushButton::clicked, this, &TwoFAManager::onAddAccount);
    connect(m_deleteBtn, &QPushButton::clicked, this, &TwoFAManager::onDeleteAccount);
    connect(m_copyBtn, &QPushButton::clicked, this, &TwoFAManager::onCopyCode);
    connect(m_accountList, &QListWidget::currentTextChanged, this, &TwoFAManager::onSelectionChanged);
}

void TwoFAManager::onAddAccount()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add 2FA Account");
    dialog.resize(400, 180);

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("e.g., GitHub, Google, Discord");

    QLineEdit* secretEdit = new QLineEdit;
    secretEdit->setPlaceholderText("Paste the secret key from the website");
    secretEdit->setEchoMode(QLineEdit::Password);

    QLineEdit* issuerEdit = new QLineEdit;
    issuerEdit->setPlaceholderText("Company name (optional)");

    layout->addRow("Account Name:", nameEdit);
    layout->addRow("Secret Key:", secretEdit);
    layout->addRow("Issuer:", issuerEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString secret = secretEdit->text().trimmed().toUpper().remove(' ');
        QString issuer = issuerEdit->text().trimmed();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "Invalid Input", "Please enter an account name.");
            return;
        }

        if (!Simple2FA::isValidSecret(secret)) {
            QMessageBox::warning(this, "Invalid Secret",
                                 "Please enter a valid secret key. It should contain only letters A-Z and numbers 2-7.");
            return;
        }

        if (m_entries.contains(name)) {
            QMessageBox::warning(this, "Duplicate Account",
                                 "An account with this name already exists.");
            return;
        }

        TwoFAEntry entry;
        entry.name = name;
        entry.secret = secret;
        entry.issuer = issuer;

        m_entries[name] = entry;
        m_accountList->addItem(name);
        m_accountList->setCurrentRow(m_accountList->count() - 1);

        saveData();
        updateCurrentCode();
    }
}

void TwoFAManager::onDeleteAccount()
{
    QString currentItem = m_accountList->currentItem() ?
                              m_accountList->currentItem()->text() : "";

    if (currentItem.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Delete Account",
                                                              QString("Are you sure you want to delete the 2FA account '%1'?").arg(currentItem),
                                                              QMessageBox::Yes | QMessageBox::No,
                                                              QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_entries.remove(currentItem);
        delete m_accountList->takeItem(m_accountList->currentRow());
        saveData();
        updateCurrentCode();
    }
}

void TwoFAManager::onCopyCode()
{
    QString code = m_currentCodeLabel->text();
    if (code != "------" && code != "000000") {
        QApplication::clipboard()->setText(code);

        // Visual feedback
        QString originalStyle = m_copyBtn->styleSheet();
        m_copyBtn->setText("Copied!");
        m_copyBtn->setStyleSheet(originalStyle + "background-color: #2196F3;");

        QTimer::singleShot(1500, [this, originalStyle]() {
            m_copyBtn->setText("Copy Code to Clipboard");
            m_copyBtn->setStyleSheet(originalStyle);
        });
    }
}

void TwoFAManager::onSelectionChanged()
{
    bool hasSelection = m_accountList->currentItem() != nullptr;
    m_deleteBtn->setEnabled(hasSelection);
    m_copyBtn->setEnabled(hasSelection);
    updateCurrentCode();
}

void TwoFAManager::updateCurrentCode()
{
    QString currentItem = m_accountList->currentItem() ?
                              m_accountList->currentItem()->text() : "";

    if (currentItem.isEmpty() || !m_entries.contains(currentItem)) {
        m_currentCodeLabel->setText("------");
        m_timeRemainingLabel->setText("Select an account");
       // m_timeProgressBar->setValue(30);
        m_timeProgressBar->setValue(Simple2FA::getTimeRemaining());

        return;
    }

    const TwoFAEntry& entry = m_entries[currentItem];
    QString code = Simple2FA::generateCode(entry.secret);
    int timeRemaining = Simple2FA::getTimeRemaining();

    m_currentCodeLabel->setText(code);
    m_timeRemainingLabel->setText(QString("Refreshes in %1 seconds").arg(timeRemaining));
    m_timeProgressBar->setValue(timeRemaining);

    updateProgressBar();
}

void TwoFAManager::updateProgressBar()
{
    int timeRemaining = Simple2FA::getTimeRemaining();
    m_timeProgressBar->setValue(timeRemaining);

    // Change color based on time remaining
    QString color;
    if (timeRemaining > 20) {
        color = "#4CAF50"; // Green
    } else if (timeRemaining > 10) {
        color = "#FFC107"; // Yellow
    } else {
        color = "#F44336"; // Red
    }

    m_timeProgressBar->setStyleSheet(
        QString("QProgressBar { border: 1px solid #ddd; border-radius: 4px; background: #f0f0f0; }"
                "QProgressBar::chunk { background: %1; border-radius: 3px; }").arg(color)
        );
}

void TwoFAManager::loadData()
{
    QSettings settings;
    settings.beginGroup("TwoFA");

    QStringList accounts = settings.childGroups();
    for (const QString& account : accounts) {
        settings.beginGroup(account);

        TwoFAEntry entry;
        entry.name = account;
        entry.secret = settings.value("secret").toString();
        entry.issuer = settings.value("issuer").toString();

        if (!entry.secret.isEmpty() && Simple2FA::isValidSecret(entry.secret)) {
            m_entries[account] = entry;
            m_accountList->addItem(account);
        }

        settings.endGroup();
    }

    settings.endGroup();
}

void TwoFAManager::saveData()
{
    QSettings settings;
    settings.beginGroup("TwoFA");
    settings.remove(""); // Clear existing data

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        const QString& name = it.key();
        const TwoFAEntry& entry = it.value();

        settings.beginGroup(name);
        settings.setValue("secret", entry.secret);
        settings.setValue("issuer", entry.issuer);
        settings.endGroup();
    }

    settings.endGroup();
}

