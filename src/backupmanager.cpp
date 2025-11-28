#include "backupmanager.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

BackupManager::BackupManager(const std::string& originalFilePath)
    : originalFilePath(originalFilePath) {}

std::string BackupManager::createBackup() {
    if (!fs::exists(originalFilePath)) {
        lastError = "Original file does not exist: " + originalFilePath;
        return "";
    }

    std::string timestamp = generateTimestamp();
    std::string backupPath = originalFilePath + ".bak" + timestamp;

    try {
        fs::copy_file(originalFilePath, backupPath, fs::copy_options::overwrite_existing);
        return backupPath;
    } catch (const std::exception& e) {
        lastError = std::string("Failed to create backup: ") + e.what();
        return "";
    }
}

std::string BackupManager::getLastBackupPath() const {
    std::vector<std::string> backups = listBackups();
    
    if (backups.empty()) {
        return "";
    }

    // Sort by modification time, most recent first
    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    
    for (const auto& backup : backups) {
        try {
            auto lastWriteTime = fs::last_write_time(backup);
            backupsWithTime.push_back({backup, lastWriteTime});
        } catch (const std::exception&) {
            continue;
        }
    }

    if (backupsWithTime.empty()) {
        return "";
    }

    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return backupsWithTime[0].first;
}

std::vector<std::string> BackupManager::listBackups() const {
    std::vector<std::string> backups;
    std::string backupPattern = getBackupBaseName();

    try {
        std::string directory = getBackupDirectory();
        
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                
                // Check if filename matches backup pattern
                if (filename.find(backupPattern) != std::string::npos) {
                    backups.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception&) {
        // Silently fail
    }

    return backups;
}

bool BackupManager::restoreFromLastBackup() {
    std::string lastBackup = getLastBackupPath();
    
    if (lastBackup.empty()) {
        lastError = "No backup found";
        return false;
    }

    return restoreFromBackup(lastBackup);
}

bool BackupManager::restoreFromBackup(const std::string& backupPath) {
    if (!fs::exists(backupPath)) {
        lastError = "Backup file does not exist: " + backupPath;
        return false;
    }

    if (!fs::exists(originalFilePath)) {
        lastError = "Original file does not exist: " + originalFilePath;
        return false;
    }

    try {
        fs::copy_file(backupPath, originalFilePath, fs::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        lastError = std::string("Failed to restore from backup: ") + e.what();
        return false;
    }
}

std::string BackupManager::getOriginalFilePath() const {
    return originalFilePath;
}

std::string BackupManager::getBackupDirectory() const {
    return fs::path(originalFilePath).parent_path().string();
}

int BackupManager::cleanupOldBackups(int keepCount) {
    if (keepCount <= 0) {
        keepCount = 1;
    }

    std::vector<std::string> backups = listBackups();
    
    if (static_cast<int>(backups.size()) <= keepCount) {
        return 0;
    }

    // Sort by modification time
    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    
    for (const auto& backup : backups) {
        try {
            auto lastWriteTime = fs::last_write_time(backup);
            backupsWithTime.push_back({backup, lastWriteTime});
        } catch (const std::exception&) {
            continue;
        }
    }

    // Sort descending (newest first)
    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    int deleted = 0;
    size_t keep = static_cast<size_t>(keepCount);
        for (size_t i = keep; i < backupsWithTime.size(); ++i) {
            try {
                fs::remove(backupsWithTime[i].first);
                deleted++;
                } catch (const std::exception&) {
        continue;
    }
}


    return deleted;
}

std::string BackupManager::getLastError() const {
    return lastError;
}

std::string BackupManager::generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    
    return ss.str();
}

std::string BackupManager::getBackupBaseName() const {
    return fs::path(originalFilePath).filename().string() + ".bak";
}

bool BackupManager::isNewer(const std::string& file1, const std::string& file2) {
    try {
        auto time1 = fs::last_write_time(file1);
        auto time2 = fs::last_write_time(file2);
        return time1 > time2;
    } catch (const std::exception&) {
        return false;
    }
}
