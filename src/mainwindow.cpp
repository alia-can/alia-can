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
#include <QCheckBox>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent), isDarkTheme(false), isTransparentEnabled(false) {
    setWindowTitle("AliaCan - Alias Manager");
    setWindowIcon(createAppIcon());
    setGeometry(100, 100, 900, 700);
    initializeShellDetection();
    initializeUI();
    setupConnections();
    loadAliasesFromFile();
    updateShellInfo();
    applyStylesheet();
    startupAnimation();
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
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QWidget* toggleContainer = new QWidget(this);
    QHBoxLayout* toggleLayout = new QHBoxLayout(toggleContainer);
    toggleLayout->setContentsMargins(0, 0, 0, 0);
    toggleLayout->setSpacing(10);
    themeToggle = new QPushButton("🌙 Dark Theme", this);
    themeToggle->setCheckable(true);
    themeToggle->setFixedSize(120, 30);
    transparencyToggle = new QPushButton("🔲 Transparency", this);
    transparencyToggle->setCheckable(true);
    transparencyToggle->setFixedSize(140, 30);
    toggleLayout->addWidget(themeToggle);
    toggleLayout->addWidget(transparencyToggle);
    toggleLayout->addStretch();
    headerLayout->addWidget(toggleContainer);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    shellInfoLabel = new QLabel(this);
    shellInfoLabel->setStyleSheet("color: #2196F3; font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(shellInfoLabel);
    QFrame* separator1 = new QFrame(this);
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);
    QGroupBox* inputGroup = new QGroupBox("Add New Alias", this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(10);
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Alias Name:", this);
    aliasNameInput = new QLineEdit(this);
    aliasNameInput->setPlaceholderText("e.g., 'll'");
    aliasNameInput->setMaximumWidth(200);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(aliasNameInput);
    nameLayout->addStretch();
    inputLayout->addLayout(nameLayout);
    QHBoxLayout* commandLayout = new QHBoxLayout();
    QLabel* commandLabel = new QLabel("Command:", this);
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("e.g., 'ls -la'");
    commandLayout->addWidget(commandLabel);
    commandLayout->addWidget(commandInput);
    inputLayout->addLayout(commandLayout);
    commandStatus = new QLabel(this);
    commandStatus->setText("");
    commandStatus->setStyleSheet("color: gray; font-size: 12px;");
    inputLayout->addWidget(commandStatus);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add Alias", this);
    addButton->setMaximumWidth(150);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    inputLayout->addLayout(buttonLayout);
    mainLayout->addWidget(inputGroup);
    QFrame* separator2 = new QFrame(this);
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);
    QWidget* searchContainer = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("🔍 Search aliases... (name or command)");
    searchInput->setClearButtonEnabled(true);
    searchLayout->addWidget(searchInput);
    mainLayout->addWidget(searchContainer);
    QGroupBox* listGroup = new QGroupBox("Current Aliases", this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setSpacing(10);
    aliasList = new QListWidget(this);
    aliasList->setMinimumHeight(250);
    listLayout->addWidget(aliasList);
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
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(themeToggle, &QPushButton::toggled, this, &MainWindow::onThemeToggled);
    connect(transparencyToggle, &QPushButton::toggled, this, &MainWindow::onTransparencyToggled);
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
    QString searchText = searchInput->text().toLower();
    for (const auto& alias : currentAliases) {
        QString aliasName = QString::fromStdString(alias.name);
        QString aliasCommand = QString::fromStdString(alias.command);
        QString displayText = aliasName + " = " + aliasCommand;
        if (searchText.isEmpty() || 
            aliasName.toLower().contains(searchText) || 
            aliasCommand.toLower().contains(searchText)) {
            aliasList->addItem(displayText);
        }
    }

    statusLabel->setText(QString("Total aliases: %1").arg(currentAliases.size()));
}

void MainWindow::onAddAlias() {
    QString aliasName = aliasNameInput->text().trimmed();
    QString command = commandInput->text().trimmed();
    if (!validateInput(aliasName, command)) {
        return;
    }

    animateButton(addButton);
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
    
    animateButton(removeButton);
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
    animateButton(refreshButton, true);
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
        commandStatus->setText("❌ Invalid command");
        commandStatus->setStyleSheet("color: red;");
    } else {
        commandStatus->setText("✔ Valid command");
        commandStatus->setStyleSheet("color: green;");
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    updateAliasList();
    if (!text.isEmpty()) {
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(searchInput);
        searchInput->setGraphicsEffect(effect);
        QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(300);
        animation->setKeyValueAt(0, 1.0);
        animation->setKeyValueAt(0.5, 0.7);
        animation->setKeyValueAt(1, 1.0);
        animation->start(QPropertyAnimation::DeleteWhenStopped);
    }
}

void MainWindow::onThemeToggled(bool checked) {
    isDarkTheme = checked;
    themeToggle->setText(checked ? "☀️ Light Theme" : "🌙 Dark Theme");
    applyStylesheet();
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(centralWidget());
    centralWidget()->setGraphicsEffect(effect);
    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(500);
    animation->setStartValue(0.3);
    animation->setEndValue(1.0);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::onTransparencyToggled(bool checked) {
    isTransparentEnabled = checked;
    transparencyToggle->setText(checked ? "🔳 No Transparency" : "🔲 Transparency");
    if (checked) {
        setWindowOpacity(0.80);
        QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
        animation->setDuration(800);
        animation->setStartValue(1.0);
        animation->setEndValue(0.95);
        animation->start();
    } else {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
        animation->setDuration(500);
        animation->setStartValue(0.95);
        animation->setEndValue(1.0);
        animation->start();
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
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(statusLabel);
    statusLabel->setGraphicsEffect(effect);
    QSequentialAnimationGroup* animationGroup = new QSequentialAnimationGroup;
    QPropertyAnimation* fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    QPropertyAnimation* fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    animationGroup->addAnimation(fadeIn);
    animationGroup->addPause(2700);
    animationGroup->addAnimation(fadeOut);
    animationGroup->start(QPropertyAnimation::DeleteWhenStopped);
    QTimer::singleShot(3000, this, [this]() {
        statusLabel->setText("");
        statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    });
}

void MainWindow::applyStylesheet() {
    QString style;
    if (isDarkTheme) {
        style = R"(
            QMainWindow {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QGroupBox {
                color: #ffffff;
                border: 2px solid #555;
                border-radius: 5px;
                margin-top: 10px;
                padding-top: 10px;
                font-weight: bold;
                background-color: #3c3c3c;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
                color: #ffffff;
            }
            QLineEdit {
                border: 1px solid #555;
                border-radius: 4px;
                padding: 6px;
                background-color: #404040;
                color: #ffffff;
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
            }
            QPushButton:hover {
                background-color: #1976D2;
            }
            QPushButton:pressed {
                background-color: #1565C0;
            }
            QPushButton:checked {
                background-color: #FF9800;
            }
            QListWidget {
                border: 1px solid #555;
                border-radius: 4px;
                background-color: #404040;
                color: #ffffff;
            }
            QListWidget::item:selected {
                background-color: #2196F3;
                color: white;
            }
            QLabel {
                color: #ffffff;
            }
            QFrame {
                color: #555;
            }
            )";
    } else {
        style = R"(
            QMainWindow {
                background-color: #f5f5f5;
                color: #000000;
            }
            QGroupBox {
                color: #333;
                border: 2px solid #ddd;
                border-radius: 5px;
                margin-top: 10px;
                padding-top: 10px;
                font-weight: bold;
                background-color: #ffffff;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
                color: #333;
            }
            QLineEdit {
                border: 1px solid #bbb;
                border-radius: 4px;
                padding: 6px;
                background-color: white;
                color: #000000;
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
            }
            QPushButton:hover {
                background-color: #1976D2;
            }
            QPushButton:pressed {
                background-color: #1565C0;
            }
            QPushButton:checked {
                background-color: #FF9800;
            }
            QListWidget {
                border: 1px solid #bbb;
                border-radius: 4px;
                background-color: white;
                color: #000000;
            }
            QListWidget::item:selected {
                background-color: #2196F3;
                color: white;
            }
            QLabel {
                color: #333;
            }
            QFrame {
                color: #ddd;
            }
            )";
    }

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

void MainWindow::animateButton(QPushButton* button, bool isRefresh) {
    QPropertyAnimation* animation = new QPropertyAnimation(button, "geometry");
    animation->setDuration(200);
    QRect originalGeometry = button->geometry();
    QRect pressedGeometry = originalGeometry;
    pressedGeometry.setWidth(originalGeometry.width() - 4);
    pressedGeometry.setHeight(originalGeometry.height() - 2);
    pressedGeometry.moveTo(originalGeometry.x() + 2, originalGeometry.y() + 1);
    animation->setKeyValueAt(0, originalGeometry);
    animation->setKeyValueAt(0.5, pressedGeometry);
    animation->setKeyValueAt(1, originalGeometry);
    if (isRefresh) {
        QGraphicsRotation* rotation = new QGraphicsRotation(button);
        rotation->setAxis(Qt::ZAxis);
        button->setGraphicsEffect(rotation);
        QPropertyAnimation* rotateAnim = new QPropertyAnimation(rotation, "angle");
        rotateAnim->setDuration(500);
        rotateAnim->setStartValue(0);
        rotateAnim->setEndValue(360);
        rotateAnim->start(QPropertyAnimation::DeleteWhenStopped);
    }
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::startupAnimation() {
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(800);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QPropertyAnimation::DeleteWhenStopped);
    QTimer::singleShot(100, [this]() {
        animateWidgetGroup(shellInfoLabel, 0);
    });
    QTimer::singleShot(200, [this]() {
        animateWidgetGroup(aliasNameInput, 1);
    });
    QTimer::singleShot(300, [this]() {
        animateWidgetGroup(commandInput, 2);
    });
    QTimer::singleShot(400, [this]() {
        animateWidgetGroup(searchInput, 3);
    });
    QTimer::singleShot(500, [this]() {
        animateWidgetGroup(aliasList, 4);
    });
}

void MainWindow::animateWidgetGroup(QWidget* widget, int delay) {
    if (!widget) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    effect->setOpacity(0.0);
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    QPropertyAnimation* fadeAnim = new QPropertyAnimation(effect, "opacity");
    fadeAnim->setDuration(500);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    QPropertyAnimation* slideAnim = new QPropertyAnimation(widget, "pos");
    slideAnim->setDuration(500);
    QPoint originalPos = widget->pos();
    slideAnim->setStartValue(originalPos + QPoint(-20, 0));
    slideAnim->setEndValue(originalPos);
    slideAnim->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(fadeAnim);
    group->addAnimation(slideAnim);
    QTimer::singleShot(delay * 100, group, &QParallelAnimationGroup::start);
    group->start(QPropertyAnimation::DeleteWhenStopped);
}
