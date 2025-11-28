#include "mainwindow.hpp"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QDialog>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), isDarkTheme(false) {
    setWindowTitle("AliaCan - Alias Manager");
    setWindowIcon(createAppIcon());
    setGeometry(100, 100, 1000, 750);
    setMinimumSize(900, 650);

    initializeShellDetection();
    initializeUI();
    setupConnections();
    loadAliasesFromFile();
    updateShellInfo();
    applyStylesheet();
}

MainWindow::~MainWindow() = default;

void MainWindow::initializeShellDetection() {
    currentShell = ShellDetector::detectShell();
    configFilePath = ShellDetector::getConfigFilePath(currentShell);
    configHandler = std::make_unique<ConfigFileHandler>(configFilePath, currentShell);
    backupManager = std::make_unique<BackupManager>(configFilePath);
}

void MainWindow::initializeUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(25, 20, 25, 20);
    mainLayout->setSpacing(18);

    // ===== TOP HEADER =====
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);

    shellInfoLabel = new QLabel(this);
    shellInfoLabel->setStyleSheet(
        "font-weight: bold; font-size: 13px; letter-spacing: 0.5px;"
    );
    headerLayout->addWidget(shellInfoLabel);
    headerLayout->addStretch();

    // Theme Toggle Button (Top Right)
    themeToggle = new QPushButton("🌙", this);
    themeToggle->setMaximumSize(40, 40);
    themeToggle->setCursor(Qt::PointingHandCursor);
    themeToggle->setStyleSheet(
        "QPushButton { border-radius: 20px; font-size: 18px; border: none; }"
    );
    headerLayout->addWidget(themeToggle);

    mainLayout->addLayout(headerLayout);

    // ===== SEARCH SECTION =====
    QVBoxLayout* searchLayout = new QVBoxLayout();
    searchLayout->setSpacing(8);

    QLabel* searchLabel = new QLabel("🔍 Search Aliases", this);
    searchLabel->setStyleSheet("font-weight: 600; font-size: 12px; letter-spacing: 0.3px;");
    searchLayout->addWidget(searchLabel);

    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Type alias name or command to filter...");
    searchInput->setMaximumHeight(38);
    searchInput->setCursor(Qt::IBeamCursor);
    searchLayout->addWidget(searchInput);

    mainLayout->addLayout(searchLayout);

    // ===== INPUT SECTION =====
    QGroupBox* inputGroup = new QGroupBox("➕ Add New Alias", this);
    inputGroup->setCursor(Qt::ArrowCursor);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(12);

    // Alias Name Input
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Alias Name:", this);
    nameLabel->setMinimumWidth(100);
    aliasNameInput = new QLineEdit(this);
    aliasNameInput->setPlaceholderText("e.g., 'll'");
    aliasNameInput->setMaximumWidth(250);
    aliasNameInput->setCursor(Qt::IBeamCursor);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(aliasNameInput);
    nameLayout->addStretch();
    inputLayout->addLayout(nameLayout);

    // Command Input
    QHBoxLayout* commandLayout = new QHBoxLayout();
    QLabel* commandLabel = new QLabel("Command:", this);
    commandLabel->setMinimumWidth(100);
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("e.g., 'ls -la'");
    commandInput->setCursor(Qt::IBeamCursor);
    commandLayout->addWidget(commandLabel);
    commandLayout->addWidget(commandInput);
    inputLayout->addLayout(commandLayout);

    // Command Status Label
    commandStatus = new QLabel(this);
    commandStatus->setText("");
    commandStatus->setStyleSheet("font-size: 11px; font-weight: 500;");
    inputLayout->addWidget(commandStatus);

    // Add Button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("✨ Add Alias", this);
    addButton->setMinimumHeight(36);
    addButton->setMaximumWidth(160);
    addButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    inputLayout->addLayout(buttonLayout);

    mainLayout->addWidget(inputGroup);

    // ===== ALIASES LIST SECTION =====
    QGroupBox* listGroup = new QGroupBox("📋 Current Aliases", this);
    listGroup->setCursor(Qt::ArrowCursor);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setSpacing(12);

    aliasList = new QListWidget(this);
    aliasList->setMinimumHeight(280);
    aliasList->setCursor(Qt::PointingHandCursor);
    listLayout->addWidget(aliasList);

    // List Control Buttons
    QHBoxLayout* listButtonLayout = new QHBoxLayout();
    listButtonLayout->setSpacing(10);

    removeButton = new QPushButton("❌ Remove", this);
    removeButton->setMinimumHeight(34);
    removeButton->setCursor(Qt::PointingHandCursor);

    refreshButton = new QPushButton("🔄 Refresh", this);
    refreshButton->setMinimumHeight(34);
    refreshButton->setCursor(Qt::PointingHandCursor);

    backupButton = new QPushButton("💾 View Backups", this);
    backupButton->setMinimumHeight(34);
    backupButton->setCursor(Qt::PointingHandCursor);

    restoreButton = new QPushButton("⚡ Restore", this);
    restoreButton->setMinimumHeight(34);
    restoreButton->setCursor(Qt::PointingHandCursor);

    listButtonLayout->addWidget(removeButton);
    listButtonLayout->addWidget(refreshButton);
    listButtonLayout->addStretch();
    listButtonLayout->addWidget(backupButton);
    listButtonLayout->addWidget(restoreButton);
    listLayout->addLayout(listButtonLayout);

    mainLayout->addWidget(listGroup);

    // ===== STATUS LABEL =====
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-size: 12px; font-weight: 500;");
    mainLayout->addWidget(statusLabel);
}

void MainWindow::setupConnections() {
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddAlias);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveAlias);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(backupButton, &QPushButton::clicked, this, &MainWindow::onShowBackups);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::onRestoreBackup);
    connect(aliasList, &QListWidget::itemSelectionChanged, this, &MainWindow::onAliasSelected);
    connect(aliasNameInput, &QLineEdit::textChanged, this, &MainWindow::onNameChanged);
    connect(commandInput, &QLineEdit::textChanged, this, &MainWindow::onCommandChanged);
    connect(themeToggle, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
}

void MainWindow::loadAliasesFromFile() {
    try {
        currentAliases = configHandler->loadAliases();
        updateAliasList();
    } catch (const std::exception& e) {
        showError("Error", QString("Failed to load aliases: ") + e.what());
    }
}

void MainWindow::updateShellInfo() {
    std::string shellName = ShellDetector::getShellName(currentShell);
    QString info = QString::fromStdString("🖥️  Detected: " + shellName + " | Config: " + configFilePath);
    shellInfoLabel->setText(info);
}

void MainWindow::updateAliasList() {
    aliasList->clear();
    for (const auto& alias : currentAliases) {
        QString item = QString::fromStdString(alias.name + " = " + alias.command);
        aliasList->addItem(item);
    }
    statusLabel->setText(QString("Total aliases: %1").arg(currentAliases.size()));
}

void MainWindow::filterAliasList(const QString& searchText) {
    for (int i = 0; i < aliasList->count(); ++i) {
        QListWidgetItem* item = aliasList->item(i);
        bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    filterAliasList(text);
}

void MainWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    themeToggle->setText(isDarkTheme ? "☀️" : "🌙");
    applyStylesheet();

    // Smooth opacity animation
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    centralWidget()->setGraphicsEffect(effect);

    QPropertyAnimation* opacityAnim = new QPropertyAnimation(effect, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(0.7);
    opacityAnim->setEndValue(1.0);
    opacityAnim->start(QAbstractAnimation::DeleteWhenDone);
}

void MainWindow::onAddAlias() {
    QString aliasName = aliasNameInput->text().trimmed();
    QString command = commandInput->text().trimmed();

    if (!validateInput(aliasName, command)) {
        return;
    }

    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }

    Alias newAlias;
    newAlias.name = aliasName.toStdString();
    newAlias.command = command.toStdString();

    if (!configHandler->addAlias(newAlias)) {
        showError("Error", QString::fromStdString("Failed to add alias: " + configHandler->getLastError()));
        return;
    }

    showSuccess("✨ Alias added successfully!");
    clearInputFields();
    loadAliasesFromFile();
}

void MainWindow::onRemoveAlias() {
    QListWidgetItem* currentItem = aliasList->currentItem();
    if (!currentItem) {
        showError("Error", "Please select an alias to remove.");
        return;
    }

    QString displayText = currentItem->text();
    QString aliasName = displayText.split(" = ")[0].trimmed();

    int reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Remove alias '%1'?").arg(aliasName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }

    if (!configHandler->removeAlias(aliasName.toStdString())) {
        showError("Error", QString::fromStdString("Failed to remove alias: " + configHandler->getLastError()));
        return;
    }

    showSuccess("❌ Alias removed successfully!");
    loadAliasesFromFile();
}

void MainWindow::onRefresh() {
    loadAliasesFromFile();
    showSuccess("🔄 Alias list refreshed!");
}

void MainWindow::onAliasSelected() {
    QListWidgetItem* currentItem = aliasList->currentItem();
    if (!currentItem) {
        return;
    }

    QString displayText = currentItem->text();
    QStringList parts = displayText.split(" = ");

    if (parts.size() == 2) {
        isModifying = true;
        aliasNameInput->setText(parts[0].trimmed());
        commandInput->setText(parts[1].trimmed());
        isModifying = false;
    }
}

void MainWindow::onNameChanged(const QString& text) {
    if (isModifying) return;
    addButton->setText(text.isEmpty() ? "✨ Add Alias" : "⚙️  Update Alias");
}

void MainWindow::onCommandChanged(const QString& text) {
    bool hasInput = !aliasNameInput->text().isEmpty() && !commandInput->text().isEmpty();
    addButton->setEnabled(hasInput);

    bool valid = AliasManager::validateCommand(text.toStdString());
    if (!valid) {
        commandStatus->setText("❌ Invalid command");
        commandStatus->setStyleSheet("color: #ff6b6b; font-size: 11px; font-weight: 500;");
    } else {
        commandStatus->setText("✅ Valid command");
        commandStatus->setStyleSheet("color: #51cf66; font-size: 11px; font-weight: 500;");
    }
}

void MainWindow::onShowBackups() {
    std::vector<std::string> backups = backupManager->listBackups();
    if (backups.empty()) {
        showError("No Backups", "No backup files found for this configuration.");
        return;
    }

    QDialog* backupDialog = new QDialog(this);
    backupDialog->setWindowTitle("Available Backups");
    backupDialog->setGeometry(150, 150, 550, 450);
    backupDialog->setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(backupDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("💾 Available Backups", backupDialog);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    layout->addWidget(titleLabel);

    QListWidget* backupList = new QListWidget(backupDialog);
    backupList->setCursor(Qt::PointingHandCursor);
    for (const auto& backup : backups) {
        backupList->addItem(QString::fromStdString(backup));
    }
    layout->addWidget(backupList);

    QLabel* hintLabel = new QLabel("⬆️ Double-click to restore a backup", backupDialog);
    hintLabel->setStyleSheet("font-size: 11px; font-style: italic;");
    layout->addWidget(hintLabel);

    connect(backupList, &QListWidget::itemDoubleClicked, [this, backupDialog, backupList]() {
        if (!backupList->currentItem()) return;
        std::string backup = backupList->currentItem()->text().toStdString();
        if (backupManager->restoreFromBackup(backup)) {
            showSuccess("⚡ Restored from backup!");
            loadAliasesFromFile();
            backupDialog->close();
        } else {
            showError("Error", QString::fromStdString("Failed to restore: " + backupManager->getLastError()));
        }
    });

    backupDialog->exec();
}

void MainWindow::onRestoreBackup() {
    std::string lastBackup = backupManager->getLastBackupPath();
    if (lastBackup.empty()) {
        showError("Error", "No backup found to restore.");
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Restore",
        "Restore from most recent backup?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (backupManager->restoreFromLastBackup()) {
            showSuccess("⚡ Restored from backup successfully!");
            loadAliasesFromFile();
        } else {
            showError("Error", QString::fromStdString("Failed to restore: " + backupManager->getLastError()));
        }
    }
}

bool MainWindow::validateInput(QString& aliasName, QString& command) {
    if (aliasName.isEmpty() || command.isEmpty()) {
        showError("Validation Error", "Please fill in both alias name and command.");
        return false;
    }

    if (!AliasManager::validateAliasName(aliasName.toStdString())) {
        showError("Invalid Alias Name",
            "Alias name must contain only alphanumeric characters, underscores, and hyphens.");
        return false;
    }

    if (!AliasManager::validateCommand(command.toStdString())) {
        showError("Invalid Command", "Command is too long or empty.");
        return false;
    }

    return true;
}

void MainWindow::clearInputFields() {
    aliasNameInput->clear();
    commandInput->clear();
    commandStatus->clear();
    searchInput->clear();
}

void MainWindow::showError(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

void MainWindow::showSuccess(const QString& message) {
    statusLabel->setText(message);
    if (isDarkTheme) {
        statusLabel->setStyleSheet("color: #51cf66; font-weight: 600; font-size: 12px;");
    } else {
        statusLabel->setStyleSheet("color: #2d9a1d; font-weight: 600; font-size: 12px;");
    }

    QTimer::singleShot(4000, this, [this]() {
        statusLabel->setText("");
        statusLabel->setStyleSheet("font-size: 12px; font-weight: 500;");
    });
}

QString MainWindow::getLightTheme() const {
    return R"(
QMainWindow {
    background-color: #f8f9fa;
}

QGroupBox {
    color: #1a1a1a;
    border: 2px solid #e0e0e0;
    border-radius: 10px;
    margin-top: 12px;
    padding-top: 12px;
    font-weight: 600;
    background-color: #ffffff;
    font-size: 12px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 5px 0 5px;
}

QLineEdit {
    border: 2px solid #e0e0e0;
    border-radius: 6px;
    padding: 8px 12px;
    background-color: #ffffff;
    selection-background-color: #2196F3;
    color: #1a1a1a;
    font-size: 13px;
}

QLineEdit:focus {
    border: 2px solid #2196F3;
    background-color: #f0f7ff;
}

QLineEdit:hover {
    border: 2px solid #90caf9;
}

QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2196F3, stop:1 #1976D2);
    color: white;
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: 600;
    font-size: 12px;
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #42a5f5, stop:1 #1565C0);
}

QPushButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1565C0, stop:1 #0d47a1);
}

QPushButton:disabled {
    background-color: #cccccc;
    color: #666666;
}

QListWidget {
    border: 2px solid #e0e0e0;
    border-radius: 6px;
    background-color: #ffffff;
    color: #1a1a1a;
}

QListWidget::item {
    padding: 8px;
    border-radius: 4px;
    margin: 2px;
}

QListWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #42a5f5, stop:1 #2196F3);
    color: white;
    border-radius: 4px;
}

QListWidget::item:hover {
    background-color: #f0f7ff;
}

QLabel {
    color: #1a1a1a;
}
    )";
}

QString MainWindow::getDarkTheme() const {
    return R"(
QMainWindow {
    background-color: #0d1117;
}

QGroupBox {
    color: #e0e0e0;
    border: 2px solid #30363d;
    border-radius: 10px;
    margin-top: 12px;
    padding-top: 12px;
    font-weight: 600;
    background-color: #161b22;
    font-size: 12px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 5px 0 5px;
}

QLineEdit {
    border: 2px solid #30363d;
    border-radius: 6px;
    padding: 8px 12px;
    background-color: #0d1117;
    selection-background-color: #1f6feb;
    color: #e0e0e0;
    font-size: 13px;
}

QLineEdit:focus {
    border: 2px solid #1f6feb;
    background-color: #0d1117;
}

QLineEdit:hover {
    border: 2px solid #388bfd;
}

QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f6feb, stop:1 #1555d6);
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: 600;
    font-size: 12px;
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #388bfd, stop:1 #1f6feb);
}

QPushButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0969da, stop:1 #0860ca);
}

QPushButton:disabled {
    background-color: #21262d;
    color: #666666;
}

QListWidget {
    border: 2px solid #30363d;
    border-radius: 6px;
    background-color: #0d1117;
    color: #e0e0e0;
}

QListWidget::item {
    padding: 8px;
    border-radius: 4px;
    margin: 2px;
}

QListWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #388bfd, stop:1 #1f6feb);
    color: white;
    border-radius: 4px;
}

QListWidget::item:hover {
    background-color: #161b22;
}

QLabel {
    color: #e0e0e0;
}
    )";
}

void MainWindow::applyStylesheet() {
    QString style = isDarkTheme ? getDarkTheme() : getLightTheme();
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(style);
}

QIcon MainWindow::createAppIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Gradient background
    QLinearGradient gradient(0, 0, 64, 64);
    gradient.setColorAt(0, QColor(33, 150, 243));
    gradient.setColorAt(1, QColor(21, 101, 192));
    painter.fillRect(0, 0, 64, 64, gradient);

    // Draw "A" icon
    QFont font;
    font.setPointSize(36);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "A");
    painter.end();

    return QIcon(pixmap);
}
