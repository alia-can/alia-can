#pragma once
#include <string>
#include "shelldetector.hpp"

struct Alias {
    std::string name;
    std::string command;
    bool operator==(const Alias& other) const {
        return name == other.name && command == other.command;
    }
};
class AliasManager {
public:
    explicit AliasManager(ShellDetector::Shell shell);
    static bool validateAliasName(const std::string& name);
    static bool validateCommand(const std::string& command);
    std::string formatAlias(const Alias& alias) const;
    static Alias parseAliasLine(const std::string& line);
    static bool isAliasLine(const std::string& line);
    ShellDetector::Shell getShell() const;
    void setShell(ShellDetector::Shell shell);
    static std::string extractQuotedString(const std::string& str, size_t start);
    static std::string escapeCommand(const std::string& command);
    static std::string unescapeString(const std::string& str);
private:
    ShellDetector::Shell currentShell;
};
