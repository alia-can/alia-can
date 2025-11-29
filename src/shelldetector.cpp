#include "shelldetector.hpp"
#include <cstdlib>
#include <utility>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <pwd.h>
#include <string_view>

namespace fs = std::filesystem;
ShellDetector::Shell ShellDetector::detectShell() {
    if (auto shell = detectFromEnvironment(); shell != Shell::UNKNOWN) return shell;
    if (auto shell = detectFromConfigFiles(); shell != Shell::UNKNOWN) return shell;
    if (std::string parent = getParentProcess(); !parent.empty()) {
        if (parent.find("zsh") != std::string::npos) return Shell::ZSH;
        if (parent.find("bash") != std::string::npos) return Shell::BASH;
        if (parent.find("fish") != std::string::npos) return Shell::FISH;
    }
    return Shell::BASH;
}
ShellDetector::Shell ShellDetector::detectFromEnvironment() {
    if (const char* shellEnv = std::getenv("SHELL"); shellEnv != nullptr) {
        std::string_view shellPath(shellEnv);
        if (shellPath.find("zsh") != std::string_view::npos) return Shell::ZSH;
        if (shellPath.find("bash") != std::string_view::npos) return Shell::BASH;
        if (shellPath.find("fish") != std::string_view::npos) return Shell::FISH;
    }
    return Shell::UNKNOWN;
}
ShellDetector::Shell ShellDetector::detectFromConfigFiles() {
    std::string home = expandHome("~");
    constexpr std::pair<Shell, std::string_view> configs[] = {
        {Shell::ZSH, ZSHRC},
        {Shell::BASH, BASHRC},
        {Shell::FISH, FISH_CONFIG}
    };
    for (const auto& [shell, config] : configs) {
        if (fs::exists(home + "/" + std::string(config)))
            return shell;
    }
    return Shell::UNKNOWN;
}
std::string ShellDetector::getParentProcess() {
    std::string procPath = "/proc/" + std::to_string(getppid()) + "/comm";
    if (std::ifstream procFile(procPath); procFile.is_open()) {
        std::string processName;
        if (std::getline(procFile, processName) && !processName.empty() && processName.back() == '\n') processName.pop_back();
        return processName;
    }
    return "";
}
std::string ShellDetector::expandHome(const std::string& path) {
    if (path.empty() || path[0] != '~') return path;
    const char* homeDir = std::getenv("HOME");
    if (homeDir == nullptr) {
        if (struct passwd* pw = getpwuid(getuid()); pw != nullptr) homeDir = pw->pw_dir;
        else return path;
    }
    return path.length() > 1 ? std::string(homeDir) + path.substr(1) : homeDir;
}
std::string ShellDetector::getConfigFilePath(Shell shell) {
    std::string home = expandHome("~");
    switch (shell) {
        case Shell::BASH: return home + "/" + std::string(BASHRC);
        case Shell::ZSH:  return home + "/" + std::string(ZSHRC);
        case Shell::FISH: return home + "/" + std::string(FISH_CONFIG);
        case Shell::UNKNOWN: return "";
    }
    return "";
}
std::string ShellDetector::getShellName(Shell shell) {
    switch (shell) {
        case Shell::BASH: return "BASH";
        case Shell::ZSH: return "ZSH";
        case Shell::FISH: return "FISH";
        case Shell::UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}
