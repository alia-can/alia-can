#include "backupmanager.hpp"
#include <cstdlib>
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
    std::string backupFileName = fs::path(originalFilePath).filename().string() + ".bak" + timestamp;
    std::string backupPath = fs::path(getBackupDirectory()) / backupFileName;

    try {
        fs::copy_file(originalFilePath, backupPath, fs::copy_options::overwrite_existing);
    } catch (const std::exception& e) {
        lastError = std::string("Failed to create backup: ") + e.what();
        return "";
    }

    cleanupAndCompressOldBackups(20);
    return backupPath;
}

int BackupManager::cleanupAndCompressOldBackups(int maxBackups) {
    if (maxBackups <= 0) maxBackups = 20;

    std::vector<std::string> backups = listBackups();

    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    for (const auto& backup : backups) {
        try { backupsWithTime.push_back({backup, fs::last_write_time(backup)}); } 
        catch (...) { continue; }
    }

    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b){ return a.second > b.second; });

    int deleted = 0;

    for (size_t i = 0; i < backupsWithTime.size(); ++i) {
        const std::string& path = backupsWithTime[i].first;

        if (i >= (size_t)maxBackups) {
            try { fs::remove(path); deleted++; } catch (...) { continue; }
        } else if (i >= 10 && i < 20) {
            if (path.substr(path.size()-3) != ".xz") {
                std::string cmd = "xz -9e " + path;
                int ret = std::system(cmd.c_str());
                if (ret != 0) lastError = "Failed to compress backup: " + path;
            }
        }
    }

    return deleted;
}

bool BackupManager::restoreFromBackup(const std::string& backupPath) {
    std::string actualBackupPath = backupPath;

    if (backupPath.substr(backupPath.size()-3) == ".xz") {
        std::string tempPath = backupPath.substr(0, backupPath.size()-3);
        std::string cmd = "xz -d -k -f " + backupPath;
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            lastError = "Failed to decompress backup: " + backupPath;
            return false;
        }
        actualBackupPath = tempPath;
    }

    if (!fs::exists(actualBackupPath)) {
        lastError = "Backup file does not exist: " + actualBackupPath;
        return false;
    }

    try {
        fs::copy_file(actualBackupPath, originalFilePath, fs::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        lastError = std::string("Failed to restore from backup: ") + e.what();
        return false;
    }
}

std::vector<std::string> BackupManager::listBackups() const {
    std::vector<std::string> backups;
    std::string backupPattern = getBackupBaseName();

    try {
        std::string directory = getBackupDirectory();

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(backupPattern) != std::string::npos) {
                    backups.push_back(entry.path().string());
                }
            }
        }
    } catch (...) {}

    return backups;
}

std::string BackupManager::getBackupDirectory() const {
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) return fs::path(originalFilePath).parent_path().string();

    fs::path backupDir = fs::path(homeDir) / ".shellbackup";

    if (!fs::exists(backupDir)) {
        try { fs::create_directories(backupDir); } catch (...) { return fs::path(originalFilePath).parent_path().string(); }
    }

    return backupDir.string();
}

std::string BackupManager::getLastBackupPath() const {
    std::vector<std::string> backups = listBackups();

    if (backups.empty()) return "";

    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    for (const auto& backup : backups) {
        try { backupsWithTime.push_back({backup, fs::last_write_time(backup)}); } 
        catch (...) { continue; }
    }

    if (backupsWithTime.empty()) return "";

    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return backupsWithTime[0].first;
}

bool BackupManager::restoreFromLastBackup() {
    std::string lastBackup = getLastBackupPath();
    if (lastBackup.empty()) {
        lastError = "No backup found";
        return false;
    }
    return restoreFromBackup(lastBackup);
}

std::string BackupManager::getOriginalFilePath() const { return originalFilePath; }

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

std::string BackupManager::getLastError() const { return lastError; }

bool BackupManager::isNewer(const std::string& file1, const std::string& file2) {
    try {
        auto time1 = fs::last_write_time(file1);
        auto time2 = fs::last_write_time(file2);
        return time1 > time2;
    } catch (...) { return false; }
}
