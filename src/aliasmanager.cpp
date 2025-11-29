#include "aliasmanager.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

AliasManager::AliasManager(ShellDetector::Shell shell) : currentShell(shell) {}
bool AliasManager::validateAliasName(const std::string& name) {
    if (name.empty() || name.length() > 255) return false;
    if (!std::isalnum(name[0]) && name[0] != '_') return false;
    return std::all_of(name.begin() + 1, name.end(), [](char c) {
        return std::isalnum(c) || c == '_' || c == '-';
    });
}
bool AliasManager::validateCommand(const std::string& command) {
    return !command.empty() && command.length() <= 2048;
}
std::string AliasManager::formatAlias(const Alias& alias) const {
    if (alias.command.find('\'') != std::string::npos) {
        return "alias " + alias.name + "=\"" + alias.command + "\"";
    }
    return "alias " + alias.name + "='" + alias.command + "'";
}
Alias AliasManager::parseAliasLine(const std::string& line) {
    Alias result;
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos || line.substr(start, 5) != "alias") return result;
    size_t eqPos = line.find('=', start + 5);
    if (eqPos == std::string::npos) return result;
    std::string namePart = line.substr(start + 5, eqPos - start - 5);
    size_t nameStart = namePart.find_first_not_of(" \t");
    size_t nameEnd = namePart.find_last_not_of(" \t");
    if (nameStart == std::string::npos) return result;
    result.name = namePart.substr(nameStart, nameEnd - nameStart + 1);
    std::string commandPart = line.substr(eqPos + 1);
    size_t cmdStart = commandPart.find_first_not_of(" \t");
    if (cmdStart == std::string::npos) return result;
    if (commandPart[cmdStart] == '\'' || commandPart[cmdStart] == '"') {
        char quote = commandPart[cmdStart];
        size_t endQuote = commandPart.find(quote, cmdStart + 1);
        result.command = endQuote != std::string::npos ?
        commandPart.substr(cmdStart + 1, endQuote - cmdStart - 1) :
        commandPart.substr(cmdStart + 1);
    } else {
        size_t commentPos = commandPart.find('#');
        result.command = commandPart.substr(cmdStart, commentPos - cmdStart);
        size_t end = result.command.find_last_not_of(" \t");
        if (end != std::string::npos) result.command.resize(end + 1);
    }
    return result;
}
bool AliasManager::isAliasLine(const std::string& line) {
    size_t start = line.find_first_not_of(" \t");
    return start != std::string::npos && line.substr(start, 5) == "alias";
}
ShellDetector::Shell AliasManager::getShell() const { return currentShell; }
void AliasManager::setShell(ShellDetector::Shell shell) { currentShell = shell; }
std::string AliasManager::extractQuotedString(const std::string& str, size_t start) {
    if (start >= str.length()) return "";
    char quote = str[start];
    size_t end = str.find(quote, start + 1);
    return end == std::string::npos ? str.substr(start + 1) : str.substr(start + 1, end - start - 1);
}
std::string AliasManager::escapeCommand(const std::string& command) {
    std::string escaped;
    escaped.reserve(command.length() * 2);
    for (char c : command) {
        if (c == '\'' || c == '"' || c == '\\' || c == '$') escaped += '\\';
        escaped += c;
    }
    return escaped;
}
std::string AliasManager::unescapeString(const std::string& str) {
    std::string unescaped;
    unescaped.reserve(str.length());
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
