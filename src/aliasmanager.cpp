#include "aliasmanager.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

AliasManager::AliasManager(ShellDetector::Shell shell) : currentShell(shell) {}

bool AliasManager::validateAliasName(const std::string& name) {
    if (name.empty() || name.length() > 255) {
        return false;
    }

    // First character must be alphanumeric or underscore
    if (!std::isalnum(name[0]) && name[0] != '_') {
        return false;
    }

    // Remaining characters can be alphanumeric, underscore, or hyphen
    for (char c : name) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }

    return true;
}

bool AliasManager::validateCommand(const std::string& command) {
    return !command.empty() && command.length() <= 2048;
}

std::string AliasManager::formatAlias(const Alias& alias) const {
    // Use single quotes as default, escape single quotes in command if needed
    std::string escaped = alias.command;
    
    // Check if we need double quotes (for commands with single quotes)
    size_t singleQuotePos = escaped.find('\'');
    
    if (singleQuotePos != std::string::npos) {
        // Use double quotes instead
        return "alias " + alias.name + "=\"" + escaped + "\"";
    } else {
        // Use single quotes (safer for most commands)
        return "alias " + alias.name + "='" + escaped + "'";
    }
}

Alias AliasManager::parseAliasLine(const std::string& line) {
    Alias result;
    
    // Trim leading whitespace
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return result; // Empty line
    }

    std::string trimmedLine = line.substr(start);

    // Check if it starts with "alias"
    if (trimmedLine.substr(0, 5) != "alias") {
        return result;
    }

    // Find the '=' character
    size_t eqPos = trimmedLine.find('=');
    if (eqPos == std::string::npos) {
        return result;
    }

    // Extract alias name (between "alias" and "=")
    std::string namePart = trimmedLine.substr(5, eqPos - 5);
    namePart.erase(0, namePart.find_first_not_of(" \t"));
    namePart.erase(namePart.find_last_not_of(" \t") + 1);

    if (namePart.empty()) {
        return result;
    }

    // Extract command (after "=")
    std::string commandPart = trimmedLine.substr(eqPos + 1);
    commandPart.erase(0, commandPart.find_first_not_of(" \t"));

    if (commandPart.empty()) {
        return result;
    }

    // Handle quoted strings
    if (commandPart[0] == '\'' || commandPart[0] == '"') {
        char quote = commandPart[0];
        size_t endQuote = commandPart.find(quote, 1);
        
        if (endQuote != std::string::npos) {
            result.command = commandPart.substr(1, endQuote - 1);
        } else {
            // Unclosed quote - take rest of line
            result.command = commandPart.substr(1);
        }
    } else {
        // Unquoted command - take until end of line or comment
        size_t commentPos = commandPart.find('#');
        if (commentPos != std::string::npos) {
            result.command = commandPart.substr(0, commentPos);
        } else {
            result.command = commandPart;
        }
        
        // Trim trailing whitespace
        size_t end = result.command.find_last_not_of(" \t");
        if (end != std::string::npos) {
            result.command = result.command.substr(0, end + 1);
        }
    }

    result.name = namePart;
    return result;
}

bool AliasManager::isAliasLine(const std::string& line) {
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return false;
    }

    return line.substr(start, 5) == "alias";
}

ShellDetector::Shell AliasManager::getShell() const {
    return currentShell;
}

void AliasManager::setShell(ShellDetector::Shell shell) {
    currentShell = shell;
}

std::string AliasManager::extractQuotedString(const std::string& str, size_t start) {
    if (start >= str.length()) {
        return "";
    }

    char quote = str[start];
    size_t end = str.find(quote, start + 1);

    if (end == std::string::npos) {
        return str.substr(start + 1);
    }

    return str.substr(start + 1, end - start - 1);
}

std::string AliasManager::escapeCommand(const std::string& command) {
    std::string escaped;
    for (char c : command) {
        if (c == '\'' || c == '"' || c == '\\' || c == '$') {
            escaped += '\\';
        }
        escaped += c;
    }
    return escaped;
}

std::string AliasManager::unescapeString(const std::string& str) {
    std::string unescaped;
    bool prevBackslash = false;

    for (char c : str) {
        if (prevBackslash) {
            unescaped += c;
            prevBackslash = false;
        } else if (c == '\\') {
            prevBackslash = true;
        } else {
            unescaped += c;
        }
    }

    return unescaped;
}
