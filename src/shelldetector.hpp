#pragma once
#include <string>
#include <string_view>

class ShellDetector {
public:
    enum class Shell { BASH, ZSH, FISH, UNKNOWN };
    static Shell detectShell();
    static std::string getConfigFilePath(Shell shell);
    static std::string getShellName(Shell shell);
    static Shell detectFromEnvironment();
    static Shell detectFromConfigFiles();
    static std::string getParentProcess();
    static std::string expandHome(const std::string& path);
private:
    static constexpr std::string_view BASHRC = ".bashrc";
    static constexpr std::string_view ZSHRC = ".zshrc";
    static constexpr std::string_view FISH_CONFIG = ".config/fish/config.fish";
};
