#include "configfilehandler.hpp"
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

namespace fs = std::filesystem;
ConfigFileHandler::ConfigFileHandler(const std::string& configFilePath, ShellDetector::Shell shell)
: configFilePath(configFilePath), shell(shell), aliasManager(shell) {}
std::vector<Alias> ConfigFileHandler::loadAliases() {
    std::vector<Alias> aliases;
    if (!configFileExists()) {
        lastError = "Config file does not exist: " + configFilePath;
        return aliases;
    }
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        lastError = "Cannot open config file for reading: " + configFilePath;
        return aliases;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (AliasManager::isAliasLine(line)) {
            if (auto parsed = AliasManager::parseAliasLine(line); !parsed.name.empty()) {
                aliases.push_back(std::move(parsed));
            }
        }
    }
    return aliases;
}
bool ConfigFileHandler::addAlias(const Alias& alias) {
    if (!AliasManager::validateAliasName(alias.name) || !AliasManager::validateCommand(alias.command)) {
        lastError = "Invalid alias name or command";
        return false;
    }
    if (!ensureFileExists()) {
        lastError = "Cannot create config file";
        return false;
    }
    std::ofstream file(configFilePath, std::ios::app);
    if (!file.is_open()) {
        lastError = "Cannot open config file for writing";
        return false;
    }
    file << '\n' << aliasManager.formatAlias(alias);
    setFilePermissions();
    return true;
}
bool ConfigFileHandler::removeAlias(const std::string& aliasName) {
    if (!configFileExists()) {
        lastError = "Config file does not exist";
        return false;
    }
    auto lines = readAllLines();
    if (lines.empty()) {
        lastError = "Failed to read config file";
        return false;
    }
    bool found = false;
    std::vector<std::string> newLines;
    newLines.reserve(lines.size());
    for (auto& line : lines) {
        if (AliasManager::isAliasLine(line)) {
            if (auto parsed = AliasManager::parseAliasLine(line); parsed.name == aliasName) {
                found = true;
                continue;
            }
        }
        newLines.push_back(std::move(line));
    }
    if (!found) {
        lastError = "Alias not found: " + aliasName;
        return false;
    }
    return writeAllLines(newLines);
}
std::string ConfigFileHandler::getConfigFilePath() const {
    switch (shell) {
        case ShellDetector::Shell::BASH: return ShellDetector::expandHome("~/.bashrc");
        case ShellDetector::Shell::ZSH: return ShellDetector::expandHome("~/.zshrc");
        case ShellDetector::Shell::FISH: return ShellDetector::expandHome("~/.config/fish/config.fish");
        default: return configFilePath;
    }
}
bool ConfigFileHandler::configFileExists() const {
    return fs::exists(configFilePath);
}
std::vector<std::string> ConfigFileHandler::readAllLines() {
    std::vector<std::string> lines;
    std::ifstream file(configFilePath);
    if (!file.is_open()) return lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(std::move(line));
    return lines;
}
bool ConfigFileHandler::writeAllLines(const std::vector<std::string>& lines) {
    std::ofstream file(configFilePath, std::ios::trunc);
    if (!file.is_open()) {
        lastError = "Cannot open file for writing";
        return false;
    }
    for (size_t i = 0; i < lines.size(); ++i) {
        file << lines[i];
        if (i < lines.size() - 1) file << '\n';
    }
    setFilePermissions();
    return true;
}
bool ConfigFileHandler::checkPermissions() const {
    struct stat sb;
    return stat(configFilePath.c_str(), &sb) == 0 && (sb.st_mode & S_IRUSR) && (sb.st_mode & S_IWUSR);
}
std::string ConfigFileHandler::getLastError() const {
    return lastError;
}
bool ConfigFileHandler::ensureFileExists() {
    if (fs::exists(configFilePath)) return true;
    std::ofstream file(configFilePath);
    if (!file.is_open()) return false;
    setFilePermissions();
    return true;
}
bool ConfigFileHandler::setFilePermissions() {
    return chmod(configFilePath.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 0;
}
