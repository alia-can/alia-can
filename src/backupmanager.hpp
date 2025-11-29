#pragma once
#include <string>
#include <filesystem>
#include <ctime>
#include <vector>

class BackupManager {
public:
    explicit BackupManager(const std::string& originalFilePath);
    std::string createBackup();
    std::string getLastBackupPath() const;
    std::vector<std::string> listBackups() const;
    bool restoreFromLastBackup();
    bool restoreFromBackup(const std::string& backupPath);
    std::string getOriginalFilePath() const;
    std::string getBackupDirectory() const;
    int cleanupOldBackups(int keepCount = 10);
    int cleanupAndCompressOldBackups(int maxBackups);
    std::string getLastError() const;
private:
    std::string originalFilePath;
    mutable std::string lastError;
    static std::string generateTimestamp();
    std::string getBackupBaseName() const;
    static bool isNewer(const std::string& file1, const std::string& file2);
};
