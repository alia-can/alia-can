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
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(25, 20, 25, 20);
    mainLayout->setSpacing(18);

    auto* headerLayout = new QHBoxLayout();
    shellInfoLabel = new QLabel(this);
    shellInfoLabel->setStyleSheet("font-weight: bold; font-size: 13px; letter-spacing: 0.5px;");
    headerLayout->addWidget(shellInfoLabel);
    headerLayout->addStretch();

    themeToggle = new QPushButton("🌙", this);
    themeToggle->setMaximumSize(40, 40);
    themeToggle->setCursor(Qt::PointingHandCursor);
    themeToggle->setStyleSheet("QPushButton { border-radius: 20px; font-size: 18px; border: none; }");
    headerLayout->addWidget(themeToggle);
    mainLayout->addLayout(headerLayout);

    auto* inputGroup = new QGroupBox("➕ Add New Alias", this);
    inputGroup->setCursor(Qt::ArrowCursor);
    auto* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(12);

    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel("Alias Name:", this);
    nameLabel->setMinimumWidth(100);
    aliasNameInput = new QLineEdit(this);
    aliasNameInput->setPlaceholderText("e.g., 'll'");
    aliasNameInput->setMaximumWidth(250);
    aliasNameInput->setCursor(Qt::IBeamCursor);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(aliasNameInput);
    nameLayout->addStretch();
    inputLayout->addLayout(nameLayout);

    auto* commandLayout = new QHBoxLayout();
    auto* commandLabel = new QLabel("Command:", this);
    commandLabel->setMinimumWidth(100);
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("e.g., 'ls -la'");
    commandInput->setCursor(Qt::IBeamCursor);
    commandLayout->addWidget(commandLabel);
    commandLayout->addWidget(commandInput);
    inputLayout->addLayout(commandLayout);

    commandStatus = new QLabel(this);
    commandStatus->setStyleSheet("font-size: 11px; font-weight: 500;");
    inputLayout->addWidget(commandStatus);

    auto* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("✨ Add Alias", this);
    addButton->setMinimumHeight(36);
    addButton->setMaximumWidth(160);
    addButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    inputLayout->addLayout(buttonLayout);
    mainLayout->addWidget(inputGroup);

    auto* searchLayout = new QVBoxLayout();
    searchLayout->setSpacing(8);
    auto* searchLabel = new QLabel("🔍 Search Aliases", this);
    searchLabel->setStyleSheet("font-weight: 600; font-size: 12px; letter-spacing: 0.3px;");
    searchLayout->addWidget(searchLabel);
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Type alias name or command to filter...");
    searchInput->setMaximumHeight(38);
    searchInput->setCursor(Qt::IBeamCursor);
    searchLayout->addWidget(searchInput);
    mainLayout->addLayout(searchLayout);

    auto* listGroup = new QGroupBox("📋 Current Aliases", this);
    listGroup->setCursor(Qt::ArrowCursor);
    auto* listLayout = new QVBoxLayout(listGroup);
    listLayout->setSpacing(12);
    aliasList = new QListWidget(this);
    aliasList->setMinimumHeight(280);
    aliasList->setCursor(Qt::PointingHandCursor);
    listLayout->addWidget(aliasList);

    auto* listButtonLayout = new QHBoxLayout();
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
    shellInfoLabel->setText(QString::fromStdString("🖥️  Detected: " + shellName + " | Config: " + configFilePath));
}

void MainWindow::updateAliasList() {
    aliasList->clear();
    for (const auto& alias : currentAliases) {
        aliasList->addItem(QString::fromStdString(alias.name + " = " + alias.command));
    }
    statusLabel->setText(QString("Total aliases: %1").arg(currentAliases.size()));
}

void MainWindow::filterAliasList(const QString& searchText) {
    for (int i = 0; i < aliasList->count(); ++i) {
        QListWidgetItem* item = aliasList->item(i);
        item->setHidden(!item->text().contains(searchText, Qt::CaseInsensitive));
    }
}

void MainWindow::onSearchTextChanged(const QString& text) { filterAliasList(text); }

void MainWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    themeToggle->setText(isDarkTheme ? "☀️" : "🌙");
    applyStylesheet();

    auto* effect = new QGraphicsOpacityEffect();
    centralWidget()->setGraphicsEffect(effect);
    auto* opacityAnim = new QPropertyAnimation(effect, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(0.7);
    opacityAnim->setEndValue(1.0);
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onAddAlias() {
    QString aliasName = aliasNameInput->text().trimmed();
    QString command = commandInput->text().trimmed();

    if (!validateInput(aliasName, command)) return;

    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }

    Alias newAlias{aliasName.toStdString(), command.toStdString()};
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

    QString aliasName = currentItem->text().split(" = ")[0].trimmed();
    if (QMessageBox::question(this, "Confirm Deletion", QString("Remove alias '%1'?").arg(aliasName), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

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
    if (!currentItem) return;

    QStringList parts = currentItem->text().split(" = ");
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
    addButton->setEnabled(!aliasNameInput->text().isEmpty() && !text.isEmpty());
    bool valid = AliasManager::validateCommand(text.toStdString());
    commandStatus->setText(valid ? "✅ Valid command" : "❌ Invalid command");
    commandStatus->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 500;").arg(valid ? "#51cf66" : "#ff6b6b"));
}

void MainWindow::onShowBackups() {
    std::vector<std::string> backups = backupManager->listBackups();
    if (backups.empty()) {
        showError("No Backups", "No backup files found for this configuration.");
        return;
    }

    auto* backupDialog = new QDialog(this);
    backupDialog->setWindowTitle("Available Backups");
    backupDialog->setGeometry(150, 150, 550, 450);
    backupDialog->setModal(true);

    auto* layout = new QVBoxLayout(backupDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("💾 Available Backups", backupDialog);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    layout->addWidget(titleLabel);

    auto* backupList = new QListWidget(backupDialog);
    backupList->setCursor(Qt::PointingHandCursor);
    for (const auto& backup : backups) backupList->addItem(QString::fromStdString(backup));
    layout->addWidget(backupList);

    auto* hintLabel = new QLabel("⬆️ Double-click to restore a backup", backupDialog);
    hintLabel->setStyleSheet("font-size: 11px; font-style: italic;");
    layout->addWidget(hintLabel);

    connect(backupList, &QListWidget::itemDoubleClicked, [this, backupDialog, backupList]() {
        if (!backupList->currentItem()) return;
        std::string backup = backupList->currentItem()->text().toStdString();
        if (backupManager->restoreFromBackup(backup)) {
            showSuccess("⚡ Restored from backup!");
            loadAliasesFromFile();
            backupDialog->close();
        } else showError("Error", QString::fromStdString("Failed to restore: " + backupManager->getLastError()));
    });

        backupDialog->exec();
}

void MainWindow::onRestoreBackup() {
    std::string lastBackup = backupManager->getLastBackupPath();
    if (lastBackup.empty()) {
        showError("Error", "No backup found to restore.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Restore", "Restore from most recent backup?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (backupManager->restoreFromLastBackup()) {
            showSuccess("⚡ Restored from backup successfully!");
            loadAliasesFromFile();
        } else showError("Error", QString::fromStdString("Failed to restore: " + backupManager->getLastError()));
    }
}

bool MainWindow::validateInput(QString& aliasName, QString& command) {
    if (aliasName.isEmpty() || command.isEmpty()) {
        showError("Validation Error", "Please fill in both alias name and command.");
        return false;
    }
    if (!AliasManager::validateAliasName(aliasName.toStdString())) {
        showError("Invalid Alias Name", "Alias name must contain only alphanumeric characters, underscores, and hyphens.");
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
    statusLabel->setStyleSheet(QString("color: %1; font-weight: 600; font-size: 12px;").arg(isDarkTheme ? "#51cf66" : "#2d9a1d"));
    QTimer::singleShot(4000, this, [this]() { statusLabel->setText(""); statusLabel->setStyleSheet("font-size: 12px; font-weight: 500;"); });
}

QString MainWindow::getLightTheme() const {
    return R"(
QMainWindow{background-color:#f8f9fa}
QGroupBox{color:#1a1a1a;border:2px solid #e0e0e0;border-radius:10px;margin-top:12px;padding-top:12px;font-weight:600;background-color:#ffffff;font-size:12px}
QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 5px 0 5px}
QLineEdit{border:2px solid #e0e0e0;border-radius:6px;padding:8px 12px;background-color:#ffffff;selection-background-color:#2196F3;color:#1a1a1a;font-size:13px}
QLineEdit:focus{border:2px solid #2196F3;background-color:#f0f7ff}
QLineEdit:hover{border:2px solid #90caf9}
QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2196F3,stop:1 #1976D2);color:white;border:none;border-radius:6px;padding:8px 16px;font-weight:600;font-size:12px}
QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #42a5f5,stop:1 #1565C0)}
QPushButton:pressed{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1565C0,stop:1 #0d47a1)}
QPushButton:disabled{background-color:#cccccc;color:#666666}
QListWidget{border:2px solid #e0e0e0;border-radius:6px;background-color:#ffffff;color:#1a1a1a}
QListWidget::item{padding:8px;border-radius:4px;margin:2px}
QListWidget::item:selected{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #42a5f5,stop:1 #2196F3);color:white;border-radius:4px}
QListWidget::item:hover{background-color:#f0f7ff}
QLabel{color:#1a1a1a})";
}

QString MainWindow::getDarkTheme() const {
    return R"(
QMainWindow{background-color:#0d1117}
QGroupBox{color:#e0e0e0;border:2px solid #30363d;border-radius:10px;margin-top:12px;padding-top:12px;font-weight:600;background-color:#161b22;font-size:12px}
QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 5px 0 5px}
QLineEdit{border:2px solid #30363d;border-radius:6px;padding:8px 12px;background-color:#0d1117;selection-background-color:#1f6feb;color:#e0e0e0;font-size:13px}
QLineEdit:focus{border:2px solid #1f6feb;background-color:#0d1117}
QLineEdit:hover{border:2px solid #388bfd}
QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1f6feb,stop:1 #1555d6);color:#ffffff;border:none;border-radius:6px;padding:8px 16px;font-weight:600;font-size:12px}
QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #388bfd,stop:1 #1f6feb)}
QPushButton:pressed{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0969da,stop:1 #0860ca)}
QPushButton:disabled{background-color:#21262d;color:#666666}
QListWidget{border:2px solid #30363d;border-radius:6px;background-color:#0d1117;color:#e0e0e0}
QListWidget::item{padding:8px;border-radius:4px;margin:2px}
QListWidget::item:selected{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #388bfd,stop:1 #1f6feb);color:white;border-radius:4px}
QListWidget::item:hover{background-color:#161b22}
QLabel{color:#e0e0e0})";
}

void MainWindow::applyStylesheet() {
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(isDarkTheme ? getDarkTheme() : getLightTheme());
}

QIcon MainWindow::createAppIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient gradient(0, 0, 64, 64);
    gradient.setColorAt(0, QColor(33, 150, 243));
    gradient.setColorAt(1, QColor(21, 101, 192));
    painter.fillRect(0, 0, 64, 64, gradient);
    QFont font;
    font.setPointSize(36);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "A");
    return QIcon(pixmap);
}
