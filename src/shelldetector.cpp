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

namespace fs = std::filesystem;

ShellDetector::Shell ShellDetector::detectShell() {
    // Phase 1: Check SHELL environment variable
    Shell shellFromEnv = detectFromEnvironment();
    if (shellFromEnv != Shell::UNKNOWN) {
        return shellFromEnv;
    }

    // Phase 2: Check config files existence
    Shell shellFromConfig = detectFromConfigFiles();
    if (shellFromConfig != Shell::UNKNOWN) {
        return shellFromConfig;
    }

    // Phase 3: Check parent process
    std::string parent = getParentProcess();
    if (!parent.empty()) {
        if (parent.find("zsh") != std::string::npos) return Shell::ZSH;
        if (parent.find("bash") != std::string::npos) return Shell::BASH;
        if (parent.find("fish") != std::string::npos) return Shell::FISH;
    }

    // Fallback to BASH
    return Shell::BASH;
}

ShellDetector::Shell ShellDetector::detectFromEnvironment() {
    const char* shellEnv = std::getenv("SHELL");
    if (shellEnv == nullptr) {
        return Shell::UNKNOWN;
    }

    std::string shellPath(shellEnv);
    
    if (shellPath.find("zsh") != std::string::npos) return Shell::ZSH;
    if (shellPath.find("bash") != std::string::npos) return Shell::BASH;
    if (shellPath.find("fish") != std::string::npos) return Shell::FISH;

    return Shell::UNKNOWN;
}

ShellDetector::Shell ShellDetector::detectFromConfigFiles() {
    std::string home = expandHome("~");
    
    // Check in priority order
    std::vector<std::pair<Shell, std::string>> configs = {
        {Shell::ZSH, home + "/" + ZSHRC},
        {Shell::BASH, home + "/" + BASHRC},
        {Shell::FISH, home + "/" + FISH_CONFIG}
    };

    for (const auto& [shell, configPath] : configs) {
        if (fs::exists(configPath)) {
            return shell;
        }
    }

    return Shell::UNKNOWN;
}

std::string ShellDetector::getParentProcess() {
    pid_t ppid = getppid();
    std::string procPath = "/proc/" + std::to_string(ppid) + "/comm";
    
    std::ifstream procFile(procPath);
    if (procFile.is_open()) {
        std::string processName;
        std::getline(procFile, processName);
        
        // Remove newline if present
        if (!processName.empty() && processName.back() == '\n') {
            processName.pop_back();
        }
        
        return processName;
    }

    return "";
}

std::string ShellDetector::expandHome(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }

    const char* homeDir = std::getenv("HOME");
    if (homeDir == nullptr) {
        // Fallback to pwd
        struct passwd* pw = getpwuid(getuid());
        if (pw == nullptr) {
            return path;
        }
        homeDir = pw->pw_dir;
    }

    std::string expanded(homeDir);
    if (path.length() > 1) {
        expanded += path.substr(1);
    }

    return expanded;
}

std::string ShellDetector::getConfigFilePath(Shell shell) {
    std::string home = expandHome("~");

    switch (shell) {
        case Shell::BASH:
            return home + "/" + BASHRC;
        case Shell::ZSH:
            return home + "/" + ZSHRC;
        case Shell::FISH:
            return home + "/" + FISH_CONFIG;
        case Shell::UNKNOWN:
            return "";
    }

    return "";
}

std::string ShellDetector::getShellName(Shell shell) {
    switch (shell) {
        case Shell::BASH:
            return "BASH";
        case Shell::ZSH:
            return "ZSH";
        case Shell::FISH:
            return "FISH";
        case Shell::UNKNOWN:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}
