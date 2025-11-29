#pragma once

#include <QMainWindow>
#include <memory>
#include <vector>
#include "shelldetector.hpp"
#include "aliasmanager.hpp"
#include "configfilehandler.hpp"
#include "backupmanager.hpp"

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

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
    void toggleTheme();
    void onSearchTextChanged(const QString& text);

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
    QPushButton* themeToggle;
    QListWidget* aliasList;
    QLabel* statusLabel;
    QLineEdit* searchInput;
    std::vector<Alias> currentAliases;
    bool isModifying = false;
    bool isDarkTheme = false;
    void initializeUI();
    void setupConnections();
    void initializeShellDetection();
    void loadAliasesFromFile();
    void updateShellInfo();
    void updateAliasList();
    void filterAliasList(const QString& searchText);
    void showError(const QString& title, const QString& message);
    void showSuccess(const QString& message);
    bool validateInput(QString& aliasName, QString& command);
    void clearInputFields();
    void applyStylesheet();
    QString getLightTheme() const;
    QString getDarkTheme() const;
    QIcon createAppIcon();
};
