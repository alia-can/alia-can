#include "configfilehandler.hpp"
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <iostream>

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
            Alias parsed = AliasManager::parseAliasLine(line);
            if (!parsed.name.empty()) {
                aliases.push_back(parsed);
            }
        }
    }

    file.close();
    return aliases;
}

bool ConfigFileHandler::addAlias(const Alias& alias) {
    // Validate before adding
    if (!AliasManager::validateAliasName(alias.name) || 
        !AliasManager::validateCommand(alias.command)) {
        lastError = "Invalid alias name or command";
        return false;
    }

    // Ensure file exists
    if (!ensureFileExists()) {
        lastError = "Cannot create config file";
        return false;
    }

    // Format and append
    std::ofstream file(configFilePath, std::ios::app);
    if (!file.is_open()) {
        lastError = "Cannot open config file for writing";
        return false;
    }

    file << "\n" << aliasManager.formatAlias(alias);
    file.close();

    // Ensure proper permissions
    setFilePermissions();

    return true;
}

bool ConfigFileHandler::removeAlias(const std::string& aliasName) {
    if (!configFileExists()) {
        lastError = "Config file does not exist";
        return false;
    }

    std::vector<std::string> lines = readAllLines();
    if (lines.empty()) {
        lastError = "Failed to read config file";
        return false;
    }

    // Find and remove the alias line
    bool found = false;
    std::vector<std::string> newLines;

    for (const auto& line : lines) {
        if (AliasManager::isAliasLine(line)) {
            Alias parsed = AliasManager::parseAliasLine(line);
            if (parsed.name == aliasName) {
                found = true;
                continue; // Skip this line (remove it)
            }
        }
        newLines.push_back(line);
    }

    if (!found) {
        lastError = "Alias not found: " + aliasName;
        return false;
    }

    // Write back to file
    if (!writeAllLines(newLines)) {
        lastError = "Failed to write config file";
        return false;
    }

    return true;
}

std::string ConfigFileHandler::getConfigFilePath() const {
    switch (shell) {
        case ShellDetector::Shell::BASH:
            return ShellDetector::expandHome("~/.bashrc");
        case ShellDetector::Shell::ZSH:
            return ShellDetector::expandHome("~/.zshrc");
        case ShellDetector::Shell::FISH:
            return ShellDetector::expandHome("~/.config/fish/config.fish");
        default:
            return configFilePath;
    }
}

bool ConfigFileHandler::configFileExists() const {
    return fs::exists(configFilePath);
}

std::vector<std::string> ConfigFileHandler::readAllLines() {
    std::vector<std::string> lines;
    
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
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
        if (i < lines.size() - 1) {
            file << "\n";
        }
    }

    file.close();
    setFilePermissions();
    return true;
}

bool ConfigFileHandler::checkPermissions() const {
    struct stat sb;
    if (stat(configFilePath.c_str(), &sb) != 0) {
        return false;
    }

    // Check if readable and writable by owner
    return (sb.st_mode & S_IRUSR) && (sb.st_mode & S_IWUSR);
}

std::string ConfigFileHandler::getLastError() const {
    return lastError;
}

bool ConfigFileHandler::ensureFileExists() {
    if (fs::exists(configFilePath)) {
        return true;
    }

    try {
        std::ofstream file(configFilePath);
        if (!file.is_open()) {
            return false;
        }
        file.close();
        setFilePermissions();
        return true;
    } catch (const std::exception& e) {
        lastError = std::string("Exception creating file: ") + e.what();
        return false;
    }
}

bool ConfigFileHandler::setFilePermissions() {
    // Set to 644: rw- r-- r--
    return chmod(configFilePath.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 0;
}
