#include "mainwindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QComboBox>
#include <QDialog>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>
#include <QDateTime>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent) {
    setWindowTitle("AliaCan - Alias Manager");
    setWindowIcon(createAppIcon());
    setGeometry(100, 100, 900, 700);

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
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Shell Info Section
    shellInfoLabel = new QLabel(this);
    shellInfoLabel->setStyleSheet("color: #2196F3; font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(shellInfoLabel);

    // Separator
    QFrame* separator1 = new QFrame(this);
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);

    // Input Section
    QGroupBox* inputGroup = new QGroupBox("Add New Alias", this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(10);

    // Alias Name Input
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Alias Name:", this);
    aliasNameInput = new QLineEdit(this);
    aliasNameInput->setPlaceholderText("e.g., 'll'");
    aliasNameInput->setMaximumWidth(200);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(aliasNameInput);
    nameLayout->addStretch();
    inputLayout->addLayout(nameLayout);

    // Command Input
    QHBoxLayout* commandLayout = new QHBoxLayout();
    QLabel* commandLabel = new QLabel("Command:", this);
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("e.g., 'ls -la'");
    commandLayout->addWidget(commandLabel);
    commandLayout->addWidget(commandInput);
    inputLayout->addLayout(commandLayout);

    // ðŸ”¥ Command Status Label (FIX)
    commandStatus = new QLabel(this);
    commandStatus->setText("");
    commandStatus->setStyleSheet("color: gray; font-size: 12px;");
    inputLayout->addWidget(commandStatus);

    // Add Button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add Alias", this);
    addButton->setMaximumWidth(150);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    inputLayout->addLayout(buttonLayout);

    mainLayout->addWidget(inputGroup);

    // Separator
    QFrame* separator2 = new QFrame(this);
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);

    // Aliases List Section
    QGroupBox* listGroup = new QGroupBox("Current Aliases", this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setSpacing(10);

    aliasList = new QListWidget(this);
    aliasList->setMinimumHeight(250);
    listLayout->addWidget(aliasList);

    // List Control Buttons
    QHBoxLayout* listButtonLayout = new QHBoxLayout();
    removeButton = new QPushButton("Remove Selected", this);
    refreshButton = new QPushButton("Refresh", this);
    backupButton = new QPushButton("View Backups", this);
    restoreButton = new QPushButton("Restore Backup", this);

    listButtonLayout->addWidget(removeButton);
    listButtonLayout->addWidget(refreshButton);
    listButtonLayout->addStretch();
    listButtonLayout->addWidget(backupButton);
    listButtonLayout->addWidget(restoreButton);
    listLayout->addLayout(listButtonLayout);

    mainLayout->addWidget(listGroup);

    // Status Label
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("color: #666; font-size: 12px;");
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
    QString info = QString::fromStdString("Detected: " + shellName + " (" + configFilePath + ")");
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

void MainWindow::onAddAlias() {
    QString aliasName = aliasNameInput->text().trimmed();
    QString command = commandInput->text().trimmed();

    if (!validateInput(aliasName, command)) {
        return;
    }

    // Create backup before modification
    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }

    // Create and add alias
    Alias newAlias;
    newAlias.name = aliasName.toStdString();
    newAlias.command = command.toStdString();

    if (!configHandler->addAlias(newAlias)) {
        showError("Error", QString::fromStdString("Failed to add alias: " + configHandler->getLastError()));
        return;
    }

    showSuccess("Alias added successfully!");
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

    showSuccess("Alias removed successfully!");
    loadAliasesFromFile();
}

void MainWindow::onRefresh() {
    loadAliasesFromFile();
    showSuccess("Alias list refreshed!");
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
    addButton->setText(text.isEmpty() ? "Add Alias" : "Update Alias");
}

void MainWindow::onCommandChanged(const QString& text) {
    bool hasInput = !aliasNameInput->text().isEmpty() && !commandInput->text().isEmpty();
    addButton->setEnabled(hasInput);

    bool valid = AliasManager::validateCommand(text.toStdString());

    if (!valid) {
        commandStatus->setText("âŒ Invalid command");
        commandStatus->setStyleSheet("color: red;");
    } else {
        commandStatus->setText("âœ” Valid command");
        commandStatus->setStyleSheet("color: green;");
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
    backupDialog->setGeometry(150, 150, 500, 400);

    QVBoxLayout* layout = new QVBoxLayout(backupDialog);

    QListWidget* backupList = new QListWidget(backupDialog);
    for (const auto& backup : backups) {
        backupList->addItem(QString::fromStdString(backup));
    }

    layout->addWidget(new QLabel("Double-click to restore:", backupDialog));
    layout->addWidget(backupList);

    connect(backupList, &QListWidget::itemDoubleClicked, [this, backupDialog, backupList]() {
        if (!backupList->currentItem()) return;

        std::string backup = backupList->currentItem()->text().toStdString();
        if (backupManager->restoreFromBackup(backup)) {
            showSuccess("Restored from backup!");
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
            showSuccess("Restored from backup successfully!");
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
}

void MainWindow::showError(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

void MainWindow::showSuccess(const QString& message) {
    statusLabel->setText(message);
    statusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");

    QTimer::singleShot(3000, this, [this]() {
        statusLabel->setText("");
        statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    });
}

void MainWindow::applyStylesheet() {
    QString style = R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QGroupBox {
            color: #333;
            border: 2px solid #ddd;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px 0 3px;
        }
        QLineEdit {
            border: 1px solid #bbb;
            border-radius: 4px;
            padding: 6px;
            background-color: white;
            selection-background-color: #2196F3;
        }
        QLineEdit:focus {
            border: 2px solid #2196F3;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
            cursor: pointer;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #1565C0;
        }
        QListWidget {
            border: 1px solid #bbb;
            border-radius: 4px;
            background-color: white;
        }
        QListWidget::item:selected {
            background-color: #2196F3;
            color: white;
        }
        QLabel {
            color: #333;
        }
        )";

        qApp->setStyle("Fusion");
        qApp->setStyleSheet(style);
}

QIcon MainWindow::createAppIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(0, 0, 64, 64, QColor(33, 150, 243));

    QFont font;
    font.setPointSize(36);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "A");

    painter.end();

    return QIcon(pixmap);
}
