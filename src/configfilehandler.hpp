#pragma once

#include <string>
#include <vector>
#include "aliasmanager.hpp"
#include "shelldetector.hpp"

/**
 * @class ConfigFileHandler
 * @brief Handles reading and writing to shell configuration files
 * 
 * Manages all file I/O operations with proper error handling and atomicity
 */
class ConfigFileHandler {
public:
    /**
     * @brief Constructor
     * @param configFilePath Path to the configuration file
     * @param shell The shell type
     */
    ConfigFileHandler(const std::string& configFilePath, ShellDetector::Shell shell);

    /**
     * @brief Loads all aliases from the configuration file
     * @return Vector of parsed aliases
     * @throws std::runtime_error if file cannot be read
     */
    std::vector<Alias> loadAliases();

    /**
     * @brief Adds a new alias to the configuration file
     * Appends the alias to the end of the file
     * @param alias The alias to add
     * @return true if successful, false otherwise
     */
    bool addAlias(const Alias& alias);

    /**
     * @brief Removes an alias from the configuration file
     * @param aliasName The name of the alias to remove
     * @return true if successful, false otherwise
     */
    bool removeAlias(const std::string& aliasName);

    /**
     * @brief Gets the configuration file path
     * @return The full path to the config file
     */
    std::string getConfigFilePath() const;

    /**
     * @brief Checks if the configuration file exists
     * @return true if file exists, false otherwise
     */
    bool configFileExists() const;

    /**
     * @brief Gets all file lines for processing
     * @return Vector of all lines in the file
     */
    std::vector<std::string> readAllLines();

    /**
     * @brief Writes lines to file
     * @param lines The lines to write
     * @return true if successful, false otherwise
     */
    bool writeAllLines(const std::vector<std::string>& lines);

    /**
     * @brief Checks file read/write permissions
     * @return true if file is readable and writable
     */
    bool checkPermissions() const;

    /**
     * @brief Gets the last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

private:
    std::string configFilePath;
    ShellDetector::Shell shell;
    std::string lastError;
    AliasManager aliasManager;

    /**
     * @brief Creates config file if it doesn't exist
     * @return true if successful, false otherwise
     */
    bool ensureFileExists();

    /**
     * @brief Sets file permissions to 644 (readable, writable by owner)
     * @return true if successful, false otherwise
     */
    bool setFilePermissions();
};
