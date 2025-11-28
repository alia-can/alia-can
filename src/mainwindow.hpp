#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <memory>
#include "shelldetector.hpp"
#include "aliasmanager.hpp"
#include "configfilehandler.hpp"
#include "backupmanager.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddAlias();
    void onRemoveAlias();
    void onRefresh();
    void onAliasSelected();
    void onNameChanged(const QString& text);
    void onCommandChanged(const QString& text);
    void onShowBackups();
    void onRestoreBackup();

private:
    std::unique_ptr<ConfigFileHandler> configHandler;
    std::unique_ptr<BackupManager> backupManager;
    ShellDetector::Shell currentShell;
    std::string configFilePath;
    QLabel* shellInfoLabel;
    QLineEdit* aliasNameInput;
    QLineEdit* commandInput;
    QLabel* commandStatus;
    QPushButton* addButton;
    QPushButton* removeButton;
    QPushButton* refreshButton;
    QPushButton* backupButton;
    QPushButton* restoreButton;
    QListWidget* aliasList;
    QLabel* statusLabel;
    std::vector<Alias> currentAliases;
    bool isModifying = false;
    void initializeUI();
    void setupConnections();
    void initializeShellDetection();
    void loadAliasesFromFile();
    void updateShellInfo();
    void updateAliasList();
    void showError(const QString& title, const QString& message);
    void showSuccess(const QString& message);
    bool validateInput(QString& aliasName, QString& command);
    void clearInputFields();
    void applyStylesheet();
    QIcon createAppIcon();
};
