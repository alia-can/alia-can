#pragma once

#include <string>
#include <vector>
#include <map>
#include "shelldetector.hpp"

/**
 * @struct Alias
 * @brief Represents a single shell alias
 */
struct Alias {
    std::string name;
    std::string command;

    bool operator==(const Alias& other) const {
        return name == other.name && command == other.command;
    }
};

/**
 * @class AliasManager
 * @brief Manages alias creation, deletion, and validation
 * 
 * Handles alias parsing, formatting, and validation according to shell rules
 */
class AliasManager {
public:
    /**
     * @brief Constructor
     * @param shell The shell type for alias formatting
     */
    explicit AliasManager(ShellDetector::Shell shell);

    /**
     * @brief Validates alias name
     * Valid names: alphanumeric, underscore, hyphen; no spaces
     * @param name The alias name to validate
     * @return true if valid, false otherwise
     */
    static bool validateAliasName(const std::string& name);

    /**
     * @brief Validates command string
     * @param command The command to validate
     * @return true if valid, false otherwise
     */
    static bool validateCommand(const std::string& command);

    /**
     * @brief Formats alias into shell syntax
     * @param alias The alias to format
     * @return Formatted alias line ready for shell config file
     */
    std::string formatAlias(const Alias& alias) const;

    /**
     * @brief Parses a line from shell config to extract alias
     * Handles: alias name='command', alias name="command", alias name=command
     * @param line The config file line
     * @return Parsed alias or empty optional if not an alias line
     */
    static Alias parseAliasLine(const std::string& line);

    /**
     * @brief Checks if a line is an alias definition
     * @param line The config file line
     * @return true if line starts with "alias"
     */
    static bool isAliasLine(const std::string& line);

    /**
     * @brief Gets the current shell
     * @return The shell type
     */
    ShellDetector::Shell getShell() const;

    /**
     * @brief Sets the shell type
     * @param shell The new shell type
     */
    void setShell(ShellDetector::Shell shell);

private:
    ShellDetector::Shell currentShell;

    /**
     * @brief Extracts quoted string content
     * Handles single and double quotes
     * @param str The string to extract from
     * @param start Position of opening quote
     * @return The unquoted content
     */
    static std::string extractQuotedString(const std::string& str, size_t start);

    /**
     * @brief Escapes special characters in command
     * @param command The command to escape
     * @return Escaped command
     */
    static std::string escapeCommand(const std::string& command);

    /**
     * @brief Unescapes special characters
     * @param str The string to unescape
     * @return Unescaped string
     */
    static std::string unescapeString(const std::string& str);
};
