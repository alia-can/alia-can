#pragma once

#include <string>
#include <optional>

/**
 * @class ShellDetector
 * @brief Detects the current shell and its configuration file
 * 
 * Supports: BASH, ZSH, and FISH shells
 * Detection order: SHELL env var → config files → parent process → fallback
 */
class ShellDetector {
public:
    enum class Shell {
        BASH,
        ZSH,
        FISH,
        UNKNOWN
    };

    /**
     * @brief Detects the current shell
     * @return The detected shell type
     */
    static Shell detectShell();

    /**
     * @brief Gets the configuration file path for the shell
     * @param shell The shell type
     * @return Full path to the config file, or empty string if not found
     */
    static std::string getConfigFilePath(Shell shell);

    /**
     * @brief Gets the shell name as a string
     * @param shell The shell type
     * @return Human-readable shell name
     */
    static std::string getShellName(Shell shell);

    /**
     * @brief Gets the shell name from SHELL environment variable
     * @return Detected shell or UNKNOWN
     */
    static Shell detectFromEnvironment();

    /**
     * @brief Checks if shell config files exist
     * @return The first shell found with existing config file
     */
    static Shell detectFromConfigFiles();

    /**
     * @brief Gets parent process name
     * @return Parent process name or empty string
     */
    static std::string getParentProcess();

    /**
     * @brief Expands ~ to home directory
     * @param path Path with ~ prefix
     * @return Expanded full path
     */
    static std::string expandHome(const std::string& path);

private:
    static constexpr const char* BASHRC = ".bashrc";
    static constexpr const char* ZSHRC = ".zshrc";
    static constexpr const char* FISH_CONFIG = ".config/fish/config.fish";
};
