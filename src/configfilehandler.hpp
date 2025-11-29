#pragma once
#include <string>
#include <vector>
#include "aliasmanager.hpp"
#include "shelldetector.hpp"

class ConfigFileHandler {
public:
    ConfigFileHandler(const std::string& configFilePath, ShellDetector::Shell shell);
    std::vector<Alias> loadAliases();
    bool addAlias(const Alias& alias);
    bool removeAlias(const std::string& aliasName);
    std::string getConfigFilePath() const;
    bool configFileExists() const;
    std::vector<std::string> readAllLines();
    bool writeAllLines(const std::vector<std::string>& lines);
    bool checkPermissions() const;
    std::string getLastError() const;
private:
    std::string configFilePath;
    ShellDetector::Shell shell;
    std::string lastError;
    AliasManager aliasManager;
    bool ensureFileExists();
    bool setFilePermissions();
};
