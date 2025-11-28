#pragma once

#include <string>
#include <filesystem>
#include <ctime>
#include <vector>

/**
 * @class BackupManager
 * @brief Manages automatic backups of shell configuration files
 * 
 * Creates timestamped backups before modifications
 * Follows naming convention: {filename}.bak{timestamp}
 */
class BackupManager {
public:
    /**
     * @brief Constructor
     * @param originalFilePath Path to the original file
     */
    explicit BackupManager(const std::string& originalFilePath);

    /**
     * @brief Creates a backup of the original file
     * Backup filename: {original}.bak{YYYYMMDD_HHMMSS}
     * @return Full path to the backup file, or empty string if failed
     */
    std::string createBackup();

    /**
     * @brief Gets the path of the most recent backup
     * @return Path to the backup file, or empty if no backup exists
     */
    std::string getLastBackupPath() const;

    /**
     * @brief Lists all backups for this file
     * @return Vector of backup file paths
     */
    std::vector<std::string> listBackups() const;

    /**
     * @brief Restores from the most recent backup
     * @return true if successful, false otherwise
     */
    bool restoreFromLastBackup();

    /**
     * @brief Restores from a specific backup
     * @param backupPath Path to the backup file
     * @return true if successful, false otherwise
     */
    bool restoreFromBackup(const std::string& backupPath);

    /**
     * @brief Gets the original file path
     * @return Original file path
     */
    std::string getOriginalFilePath() const;

    /**
     * @brief Gets the backup directory
     * @return Directory where backups are stored (same as original)
     */
    std::string getBackupDirectory() const;

    /**
     * @brief Cleans up old backups (keeps latest N backups)
     * @param keepCount Number of recent backups to keep (default: 10)
     * @return Number of backups deleted
     */
    int cleanupOldBackups(int keepCount = 10);
    int cleanupAndCompressOldBackups(int maxBackups);

    /**
     * @brief Gets the last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

private:
    std::string originalFilePath;
    std::string lastError;

    /**
     * @brief Generates timestamp in format YYYYMMDD_HHMMSS
     * @return Timestamp string
     */
    static std::string generateTimestamp();

    /**
     * @brief Gets the base backup filename
     * @return Filename like "bashrc.bak"
     */
    std::string getBackupBaseName() const;

    /**
     * @brief Compares file modification times
     * @param file1 First file path
     * @param file2 Second file path
     * @return true if file1 is newer than file2
     */
    static bool isNewer(const std::string& file1, const std::string& file2);
};
